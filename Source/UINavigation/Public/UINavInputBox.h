// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once

#include "UINavComponent.h"
#include "Blueprint/UserWidget.h"
#include "Data/InputRebindData.h"
#include "Data/InputRestriction.h"
#include "Data/InputType.h"
#include "Data/RevertRebindReason.h"
#include "UINavInputBox.generated.h"

#define IS_AXIS (AxisType != EAxisType::None)

class UUINavInputComponent;
class UInputAction;
class UInputMappingContext;
class UInputSettings;
class UTextBlock;
class URichTextBlock;
struct FInputAxisKeyMapping;

/**
* This class contains the logic for rebinding input keys to their respective actions
*/
UCLASS()
class UINAVIGATION_API UUINavInputBox : public UUINavComponent
{
	GENERATED_BODY()
	
protected:
	FKey CurrentKey = FKey();
	bool bUsingKeyDisplay;

	bool bAwaitingNewKey = false;
	FKey AwaitingNewKey = FKey();

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;

	FText GetKeyText();
	void UpdateKeyDisplay();
	void ProcessInputName(const UInputAction* Action);

	FText ActionDisplayName;

	UFUNCTION()
	void InputComponentClicked();

public:

	UUINavInputBox(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	void ResetToDefault();
	
	UFUNCTION()
	void OnInputTypeChanged(EInputType InputType);
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void CreateEnhancedInputKeyWidgets();

	void CreateKeyWidgets();
	bool TrySetupNewKey(const FKey& NewKey);
	void ResetKeyWidgets();
	void UpdateInputKey(const FKey& NewKey, const bool bSkipChecks = false);
	void FinishUpdateNewKey();
	void FinishUpdateNewEnhancedInputKey(const FKey& PressedKey);
	void CancelUpdateInputKey(const ERevertRebindReason Reason);
	void RevertToKeyText();
	ERevertRebindReason CanRegisterKey(class UUINavInputBox* InputBox, const FKey NewKey, int& OutCollidingActionIndex);

	FText GetCurrentText() const;

	bool ContainsKey(const FKey& CompareKey) const;
	FORCEINLINE FKey GetKey() { return CurrentKey; }

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UINav Input")
	class UUINavInputDisplay* InputDisplay = nullptr;

	UPROPERTY()
	class UUINavPCComponent* UINavPC = nullptr;

	UPROPERTY()
	FName InputName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TArray<int> EnhancedInputGroups;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	EInputRestriction InputRestriction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FName PlayerMappableKeySettingsName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TArray<FName> MirrorToPlayerMappableKeySettingsNames;

	//The text used for empty key buttons
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FText EmptyKeyText = FText::FromString(TEXT("Unbound"));

	//The text used for notifying the player to press a key
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	FText PressKeyText = FText::FromString(TEXT("Press Any Key"));

	/*
	A list of the keys that the player should only be able to use for the inputs
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TArray<FKey> KeyWhitelist;
	
	/*
	A list of the keys that the player shouldn't be able to use for the inputs.
	Only used if KeyWhitelist is empty.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UINav Input")
	TArray<FKey> KeyBlacklist =
	{
		EKeys::Escape,
		EKeys::LeftCommand,
		EKeys::RightCommand,
	};

	/*
	*	Called when key was successfully rebinded
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
	void OnKeyRebinded(FName ActionInputName, FKey OldKey, FKey NewKey);

	virtual void OnKeyRebinded_Implementation(FName ActionInputName, FKey OldKey, FKey NewKey) {};

	/*
	*	Called when a rebind was cancelled, specifying the reason for the revert
	*/
	UFUNCTION(BlueprintNativeEvent, Category = UINavWidget)
	void OnRebindCancelled(ERevertRebindReason RevertReason, FKey PressedKey);

	virtual void OnRebindCancelled_Implementation(ERevertRebindReason RevertReason, FKey PressedKey) {};

	FInputRebindData InputData = FInputRebindData();
};
