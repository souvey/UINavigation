// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavInputBox.h"

#include "EnhancedInputSubsystems.h"
#include "UINavInputComponent.h"
#include "UINavInputContainer.h"
#include "UINavMacros.h"
#include "UINavSettings.h"
#include "UINavPCComponent.h"
#include "UINavWidget.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/Image.h"
#include "Data/RevertRebindReason.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GameFramework/InputSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "PlayerMappableKeySettings.h"
#include "UINavInputDisplay.h"
#include "UserSettings/EnhancedInputUserSettings.h"

UUINavInputBox::UUINavInputBox(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsFocusable(false);
}

void UUINavInputBox::NativeConstruct()
{
	Super::NativeConstruct();

	UINavPC = Cast<APlayerController>(GetOwningPlayer())->FindComponentByClass<UUINavPCComponent>();
	if (IsValid(UINavPC))
	{
		UINavPC->InputTypeChangedDelegate.AddUniqueDynamic(this, &UUINavInputBox::OnInputTypeChanged);
	}

	OnClicked.RemoveAll(this);
	OnClicked.AddDynamic(this, &UUINavInputBox::InputComponentClicked);

	CreateKeyWidgets();
;}

void UUINavInputBox::NativeDestruct()
{
	Super::NativeDestruct();

	if (IsValid(UINavPC))
	{
		UINavPC->InputTypeChangedDelegate.RemoveDynamic(this, &UUINavInputBox::OnInputTypeChanged);
	}
}

void UUINavInputBox::CreateKeyWidgets()
{
	CreateEnhancedInputKeyWidgets();
}

void UUINavInputBox::CreateEnhancedInputKeyWidgets()
{
	if (!IsValid(UINavPC) || !IsValid(UINavPC->GetPC()) || !IsValid(UINavPC->GetPC()->GetLocalPlayer()))
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* PlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(UINavPC->GetPC()->GetLocalPlayer());
	if (!IsValid(PlayerSubsystem))
	{
		return;
	}
	UEnhancedInputUserSettings* PlayerSettings = PlayerSubsystem->GetUserSettings();
	if (!IsValid(PlayerSettings))
	{
		return;
	}
	const FPlayerKeyMapping* KeyMapping = PlayerSettings->FindCurrentMappingForSlot(PlayerMappableKeySettingsName, EPlayerMappableKeySlot::First);
	if (KeyMapping != nullptr && IsValid(KeyMapping->GetAssociatedInputAction()))
	{
		ProcessInputName(KeyMapping->GetAssociatedInputAction());
	}
	if (KeyMapping != nullptr && KeyMapping->GetCurrentKey().IsValid())
	{
		TrySetupNewKey(KeyMapping->GetCurrentKey());
	} else
	{
		SetText(EmptyKeyText);
		InputDisplay->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(NavText)) NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(NavRichText)) NavRichText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CurrentKey = FKey();
	}
}

bool UUINavInputBox::TrySetupNewKey(const FKey& NewKey)
{
	if (!NewKey.IsValid()) return false;
	CurrentKey = NewKey;
	
	bUsingKeyDisplay = true;
	InputDisplay->OverrideWithExactKey = CurrentKey;
	InputDisplay->UpdateInputVisuals();
	InputDisplay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (IsValid(NavText)) NavText->SetVisibility(ESlateVisibility::Collapsed);
	if (IsValid(NavRichText)) NavRichText->SetVisibility(ESlateVisibility::Collapsed);
	SetText(GetKeyText());

	return true;
}

void UUINavInputBox::OnInputTypeChanged(EInputType InputType)
{
	ResetKeyWidgets();
}

void UUINavInputBox::ResetKeyWidgets()
{
	CurrentKey = FKey();
	bUsingKeyDisplay = false;
	CreateKeyWidgets();
}

void UUINavInputBox::UpdateInputKey(const FKey& NewKey, const bool bSkipChecks)
{
	AwaitingNewKey = NewKey;

	if (!bSkipChecks)
	{
		int CollidingActionIndex = INDEX_NONE;
		const ERevertRebindReason RevertReason = CanRegisterKey(this, NewKey, CollidingActionIndex);
		if (RevertReason != ERevertRebindReason::None)
		{
			CancelUpdateInputKey(RevertReason);
			return;
		}
	}

	return FinishUpdateNewKey();
}

void UUINavInputBox::FinishUpdateNewKey()
{
	const FKey OldKey = CurrentKey;
	FinishUpdateNewEnhancedInputKey(AwaitingNewKey);
	OnKeyRebinded(InputName, OldKey, CurrentKey);
	bAwaitingNewKey = false;
	if (UINavPC->IsListeningToInputRebind())
	{
		UINavPC->CancelRebind();
	}
}

void UUINavInputBox::FinishUpdateNewEnhancedInputKey(const FKey& PressedKey)
{

	if (!IsValid(UINavPC) || !IsValid(UINavPC->GetPC()) || !IsValid(UINavPC->GetPC()->GetLocalPlayer()))
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* PlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(UINavPC->GetPC()->GetLocalPlayer());
	if (!IsValid(PlayerSubsystem))
	{
		return;
	}
	UEnhancedInputUserSettings* PlayerSettings = PlayerSubsystem->GetUserSettings();
	if (!IsValid(PlayerSettings))
	{
		return;
	}
	FMapPlayerKeyArgs Args = {};
	Args.MappingName = PlayerMappableKeySettingsName;
	Args.Slot = EPlayerMappableKeySlot::First;
	Args.NewKey = PressedKey;
	FGameplayTagContainer FailureReason;
	PlayerSettings->MapPlayerKey(Args, FailureReason);
	if (FailureReason.IsValid())
	{
		FString Message = TEXT("Failed to rebind ");
		Message.Append(*InputName.ToString());
		Message.Append(TEXT(": "));
		Message.Append(FailureReason.ToStringSimple(true));
		DISPLAYERROR(Message);
		return;
	}
	for (const FName Mirror : MirrorToPlayerMappableKeySettingsNames)
	{
		Args.MappingName = Mirror;
		PlayerSettings->MapPlayerKey(Args, FailureReason);
		if (FailureReason.IsValid())
		{
			FString Message = TEXT("Failed to mirror to ");
			Message.Append(*Mirror.ToString());
			Message.Append(TEXT(": "));
			Message.Append(FailureReason.ToStringSimple(true));
			DISPLAYERROR(Message);
			return;
		}
	}
	
	PlayerSettings->ApplySettings();
	PlayerSettings->AsyncSaveSettings();

	UINavPC->RequestRebuildMappings();

	CurrentKey = PressedKey;
	SetText(GetKeyText());
	UpdateKeyDisplay();
}

void UUINavInputBox::CancelUpdateInputKey(const ERevertRebindReason Reason)
{
	if (!bAwaitingNewKey)
	{
		return;
	}

	OnRebindCancelled(Reason, AwaitingNewKey);
	RevertToKeyText();
	bAwaitingNewKey = false;
	if (UINavPC->IsListeningToInputRebind())
	{
		UINavPC->CancelRebind();
	}
}

void UUINavInputBox::ProcessInputName(const UInputAction* Action)
{
	InputName = Action->GetFName();
	if (UPlayerMappableKeySettings* Settings = Action->GetPlayerMappableKeySettings().Get(); IsValid(Settings))
	{
		if (Settings->DisplayName.IsEmpty())
		{
			ActionDisplayName = FText::FromName(InputName);
		} else
		{
			ActionDisplayName = Settings->DisplayName;
		}
	}
}

void UUINavInputBox::InputComponentClicked()
{
	bAwaitingNewKey = true;

	SetText(PressKeyText);

	if (bUsingKeyDisplay)
	{
		InputDisplay->SetVisibility(ESlateVisibility::Collapsed);
		
		if (IsValid(NavText)) NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(NavRichText)) NavRichText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	UINavPC->ListenToInputRebind(this);
}

void UUINavInputBox::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (IsValid(UINavPC))
	{
		UINavPC->CancelRebind();	
	}
}

void UUINavInputBox::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);
	if (IsValid(UINavPC))
	{
		UINavPC->CancelRebind();	
	}
}

FText UUINavInputBox::GetKeyText()
{
	const FKey Key = CurrentKey;
	return UINavPC->GetKeyText(Key);
}

void UUINavInputBox::UpdateKeyDisplay()
{
	InputDisplay->OverrideWithExactKey = CurrentKey;
	InputDisplay->UpdateInputVisuals();
	bUsingKeyDisplay = CurrentKey.IsValid();
	if (bUsingKeyDisplay)
	{
		InputDisplay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(NavText)) NavText->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(NavRichText)) NavRichText->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		InputDisplay->SetVisibility(ESlateVisibility::Collapsed);
		if (IsValid(NavText)) NavText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (IsValid(NavRichText)) NavRichText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UUINavInputBox::RevertToKeyText()
{
	FText OldName;
	if (!(CurrentKey.GetFName().IsEqual(FName("None"))))
	{
		OldName = GetKeyText();
		UpdateKeyDisplay();
	}
	else
	{
		OldName = EmptyKeyText;
	}

	SetText(OldName);
}

ERevertRebindReason UUINavInputBox::CanRegisterKey(class UUINavInputBox* InputBox, const FKey NewKey, int& OutCollidingActionIndex)
{
	if (!NewKey.IsValid()) return ERevertRebindReason::BlacklistedKey;
	if (KeyWhitelist.Num() > 0 && !KeyWhitelist.Contains(NewKey)) return ERevertRebindReason::NonWhitelistedKey;
	if (KeyBlacklist.Contains(NewKey)) return ERevertRebindReason::BlacklistedKey;
	if (!UUINavBlueprintFunctionLibrary::RespectsRestriction(NewKey, InputBox->InputRestriction)) return ERevertRebindReason::RestrictionMismatch;
	if (InputBox->ContainsKey(NewKey)) return ERevertRebindReason::UsedBySameInput;

	return ERevertRebindReason::None;
}

FText UUINavInputBox::GetCurrentText() const
{
	return ActionDisplayName;
}

bool UUINavInputBox::ContainsKey(const FKey& CompareKey) const
{
	return CurrentKey == CompareKey;
}
