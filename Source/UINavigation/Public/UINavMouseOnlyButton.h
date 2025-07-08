// 

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Data/InputType.h"
#include "UINavMouseOnlyButton.generated.h"

class UUINavPCComponent;
/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavMouseOnlyButton : public UButton
{
	GENERATED_BODY()

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UFUNCTION()
	void OnInputTypeChanged(EInputType InputType);

private:
	UPROPERTY()
	FButtonStyle OriginalWidgetStyle;	
};
