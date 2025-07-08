// 


#include "UINavMouseOnlyButton.h"

#include "UINavPCComponent.h"

TSharedRef<SWidget> UUINavMouseOnlyButton::RebuildWidget()
{
	OriginalWidgetStyle = GetStyle();
	if (APlayerController* PC = Cast<APlayerController>(GetOwningPlayer()))
	{
		if (UUINavPCComponent* UINavPC = PC->FindComponentByClass<UUINavPCComponent>())
		{
			UINavPC->InputTypeChangedDelegate.AddDynamic(this, &UUINavMouseOnlyButton::OnInputTypeChanged);
		}
	}
	return Super::RebuildWidget();
}

void UUINavMouseOnlyButton::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	if (APlayerController* PC = Cast<APlayerController>(GetOwningPlayer()))
	{
		if (UUINavPCComponent* UINavPC = PC->FindComponentByClass<UUINavPCComponent>())
		{
			UINavPC->InputTypeChangedDelegate.RemoveDynamic(this, &UUINavMouseOnlyButton::OnInputTypeChanged);
		}
	}
	OriginalWidgetStyle = {};
}

void UUINavMouseOnlyButton::OnInputTypeChanged(EInputType InputType)
{
	if (InputType == EInputType::Mouse)
	{
		SetStyle(OriginalWidgetStyle);
	} else
	{
		FButtonStyle NoMouse = OriginalWidgetStyle;
		NoMouse.Hovered = NoMouse.Normal;
		NoMouse.Pressed = NoMouse.Normal;
		SetStyle(NoMouse);
	}
}
