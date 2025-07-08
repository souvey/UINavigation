// Copyright (C) 2023 Gon�alo Marques - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "UINavButtonBase.generated.h"

class SUINavButton;

/**
 * 
 */
UCLASS()
class UINAVIGATION_API UUINavButtonBase : public UButton
{
	GENERATED_BODY()
	
public:
	void SetIsFocusable(const bool bInIsButtonFocusable);
	bool SimulatePress();

protected:

	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// End of UWidget interface

protected:

	/** Cached pointer to the underlying slate button owned by this UWidget */
	TSharedPtr<SUINavButton> MyUINavButton;
};
