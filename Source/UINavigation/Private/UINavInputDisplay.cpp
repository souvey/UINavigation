// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved


#include "UINavInputDisplay.h"

#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "UINavPCComponent.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "UINavSettings.h"
#include "UINavBlueprintFunctionLibrary.h"
#include "UINavMacros.h"
#include "Data/InputIconMapping.h"
#include "Engine/Font.h"

void UUINavInputDisplay::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (!IsValid(PC))
	{
		return;
	}

	UINavPC = PC->FindComponentByClass<UUINavPCComponent>();
	if (!IsValid(UINavPC))
	{
		return;
	}

	UINavPC->UpdateInputIconsDelegate.AddDynamic(this, &UUINavInputDisplay::UpdateInputVisuals);
	
	UpdateInputVisuals();
}

void UUINavInputDisplay::NativeDestruct()
{
	if (!IsValid(UINavPC))
	{
		return;
	}

	UINavPC->UpdateInputIconsDelegate.RemoveDynamic(this, &UUINavInputDisplay::UpdateInputVisuals);

	Super::NativeDestruct();
}

void UUINavInputDisplay::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsValid(InputText))
	{
		if (bOverride_Font)
		{
			InputText->SetFont(FontOverride);
		}
		else
		{
			FontOverride = InputText->GetFont();
		}

		if (bOverride_TextColor)
		{
			InputText->SetColorAndOpacity(TextColorOverride);
		}
		else
		{
			TextColorOverride = InputText->GetColorAndOpacity();
		}
	}

	if (IsValid(InputRichText))
	{
		InputRichText->SetText(StyleRowName.IsEmpty() ? InputRichText->GetText() : UUINavBlueprintFunctionLibrary::ApplyStyleRowToText(InputRichText->GetText(), StyleRowName));
	}

	if (IsValid(InputImage) && !bMatchIconSize)
	{
		InputImage->SetDesiredSizeOverride(IconSize);
	}
}

void UUINavInputDisplay::UpdateInputVisuals()
{
	if (!IsValid(UINavPC))
	{
		return;
	}

	if (!IsValid(InputImage))
	{
		DISPLAYERROR(TEXT("Use a UINavInputDisplay widget Blueprint class instead of the C++ class directly!"));
		return;
	}

	if (IsValid(InputText)) InputText->SetVisibility(ESlateVisibility::Collapsed);
	if (IsValid(InputRichText)) InputRichText->SetVisibility(ESlateVisibility::Collapsed);
	if (IsValid(InputIconText)) InputIconText->SetVisibility(ESlateVisibility::Collapsed);
	InputImage->SetVisibility(ESlateVisibility::Collapsed);

	FKey Key;
	if (OverrideWithExactKey.IsValid())
	{
		Key = OverrideWithExactKey;
	} else if (!OverrideWithPlayerMappableKeySettingsName.IsNone())
	{
		Key = UINavPC->GetEnhancedInputMappableKey(OverrideWithPlayerMappableKeySettingsName);
	} else if (InputTypeRestriction != EInputRestriction::None)
	{
		Key = UINavPC->GetEnhancedInputKey(InputAction, Axis, Scale, InputTypeRestriction);
	} else if (UINavPC->IsUsingGamepad())
	{
		Key = UINavPC->GetEnhancedInputKey(InputAction, Axis, Scale, EInputRestriction::Gamepad);
	} else
	{
		// Prefer matching the exact current input device, but allow fallback between mouse and keyboard.
		Key = UINavPC->GetEnhancedInputKey(InputAction, Axis, Scale, UINavPC->IsUsingMouse() ? EInputRestriction::Mouse : EInputRestriction::Keyboard);
		if (!Key.IsValid()) Key = UINavPC->GetEnhancedInputKey(InputAction, Axis, Scale, EInputRestriction::Keyboard_Mouse);
	}

	FInputIconMapping Icon = UINavPC->GetKeyIconMapping(Key);
	UFont* Font = Icon.InputIconFont.LoadSynchronous();
	if (DisplayType != EInputDisplayType::Text && IsValid(Font) && IsValid(InputIconText))
	{
		
		FSlateFontInfo FontInfo(Font, InputIconText->GetFont().Size);
		FontInfo.OutlineSettings = InputIconText->GetFont().OutlineSettings;
		InputIconText->SetFont(FontInfo);
		InputIconText->SetText(FText::FromString(Icon.InputIconFontTextSolid));
		InputIconText->SetVisibility(ESlateVisibility::Visible);
		return;
	}
	
	TSoftObjectPtr<UTexture2D> NewSoftTexture = GetDefault<UUINavSettings>()->bLoadInputIconsAsync ?
		UINavPC->GetSoftKeyIcon(Key) : UINavPC->GetKeyIcon(Key);
		
	if (!NewSoftTexture.IsNull() && DisplayType != EInputDisplayType::Text)
	{
		InputImage->SetBrushFromSoftTexture(NewSoftTexture, bMatchIconSize);
		if (!bMatchIconSize)
		{
			InputImage->SetDesiredSizeOverride(IconSize);
		}

		InputImage->SetVisibility(ESlateVisibility::Visible);
	}
	if (NewSoftTexture.IsNull() || DisplayType != EInputDisplayType::Icon)
	{
		const FText InputRawText = UINavPC->GetKeyText(Key);
		if (IsValid(InputText))
		{
			// NOTE(souvey): this is a hack due to how I'm currently constructing the visuals for keys.
			// I should probably figure out where to move this into game-specific code, rather than this plugin.
			FString Text = InputRawText.ToUpper().ToString();
			bool bValid = true;
			for (const TCHAR Character : Text)
			{
				bool bUpperLetter = Character >= 65 && Character <= 90;
				bool bDigit = Character >= 48 && Character <= 57;
				bool bSpace = Character == 32;
				if (!bUpperLetter && !bDigit && !bSpace)
				{
					bValid = false;
					break;
				}
			}
			if (!bValid || Text.IsEmpty())
			{
				Text = "";
				bool bWasLower = false;
				for (const TCHAR Character : Key.GetFName().ToString())
				{
					bool bUpperLetter = Character >= 65 && Character <= 90;
					bool bLowerLetter = Character >= 97 && Character <= 122;
					bool bDigit = Character >= 48 && Character <= 57;
					bool bSpace = Character == 32;
					if (bUpperLetter)
					{
						if (bWasLower)
						{
							Text.Append(TEXT(" "));
						}
						Text.AppendChar(Character);
					} else if (bLowerLetter)
					{
						Text.AppendChar(Character - 32);
					} else if (bDigit || bSpace)
					{
						Text.AppendChar(Character);
					}
					bWasLower = bLowerLetter;
				}
			}
			InputText->SetText(FText::FromString(Text));
			InputText->SetVisibility(ESlateVisibility::Visible);
		}

		if (IsValid(InputRichText))
		{
			InputRichText->SetText(StyleRowName.IsEmpty() ? InputRawText : UUINavBlueprintFunctionLibrary::ApplyStyleRowToText(InputRawText, StyleRowName));
			InputRichText->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UUINavInputDisplay::SetInputAction(UInputAction* NewAction, const EInputAxis NewAxis, const EAxisType NewScale)
{
	InputAction = NewAction;
	Axis = NewAxis;
	Scale = NewScale;

	UpdateInputVisuals();
}

void UUINavInputDisplay::SetIconSize(const FVector2D& NewSize)
{
	IconSize = NewSize;

	if (IsInViewport())
	{
		UpdateInputVisuals();
	}
}
