// Copyright (C) 2023 Gon�alo Marques - All Rights Reserved

#include "SUINavButton.h"

void SUINavButton::SetIsButtonFocusable(bool bInIsButtonFocusable)
{
	SetIsFocusable(bInIsButtonFocusable);
}

bool SUINavButton::SimulatePress()
{
	if (IsEnabled())
	{
		ExecuteOnClick();
		return true;
	}
	return false;
}

FReply SUINavButton::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Accept)
	{
		return SBorder::OnKeyUp(MyGeometry, InKeyEvent);
	}
	return SButton::OnKeyUp(MyGeometry, InKeyEvent);
}

FReply SUINavButton::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Accept)
	{
		return SBorder::OnKeyDown(MyGeometry, InKeyEvent);
	}
	return SButton::OnKeyDown(MyGeometry, InKeyEvent);
}
