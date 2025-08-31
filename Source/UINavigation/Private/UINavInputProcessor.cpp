// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#include "UINavInputProcessor.h"
#include "UINavPCComponent.h"

void FUINavInputProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
}

bool FUINavInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (UINavPC != nullptr)
	{
		if (UINavPC->IsListeningToInputRebind())
		{
			UINavPC->ProcessRebind(InKeyEvent);
			return true;
		}
		UINavPC->HandleKeyDownEvent(SlateApp, InKeyEvent);
	}

	return IInputProcessor::HandleKeyDownEvent(SlateApp, InKeyEvent);
}

bool FUINavInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (UINavPC != nullptr)
	{
		UINavPC->HandleKeyUpEvent(SlateApp, InKeyEvent);
	}

	return IInputProcessor::HandleKeyUpEvent(SlateApp, InKeyEvent);
}

bool FUINavInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
	if (UINavPC != nullptr)
	{
		UINavPC->HandleAnalogInputEvent(SlateApp, InAnalogInputEvent);
	}

	return IInputProcessor::HandleAnalogInputEvent(SlateApp, InAnalogInputEvent);
}

bool FUINavInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (UINavPC != nullptr)
	{
		UINavPC->HandleMouseMoveEvent(SlateApp, MouseEvent);
	}

	return IInputProcessor::HandleMouseMoveEvent(SlateApp, MouseEvent);
}

bool FUINavInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (UINavPC != nullptr)
	{
		if (UINavPC->IsListeningToInputRebind())
		{
			const FKeyEvent MouseKeyEvent(MouseEvent.GetEffectingButton(), FModifierKeysState(), MouseEvent.GetUserIndex(), MouseEvent.IsRepeat(), 0, 0);
			UINavPC->ProcessRebind(MouseKeyEvent);
			return true;
		}
		UINavPC->HandleMouseButtonDownEvent(SlateApp, MouseEvent);
	}

	return IInputProcessor::HandleMouseButtonDownEvent(SlateApp, MouseEvent);
}

bool FUINavInputProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (UINavPC != nullptr)
	{
		UINavPC->HandleMouseButtonUpEvent(SlateApp, MouseEvent);
	}

	return IInputProcessor::HandleMouseButtonUpEvent(SlateApp, MouseEvent);
}

bool FUINavInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGesture)
{
	if (InGesture != nullptr)
	{
		// FSceneViewport::OnTouchGesture (for some reason???) steals keyboard focus on any gesture.
		// This breaks UINavigation in all sorts of unfortunate ways.
		// Rather than attempt to fix the focus afterwards, just prevent this from happening at all,
		// since there's no need for gestures in anything I'm doing.
		if (InGesture->GetGestureType() == EGestureEvent::Scroll) {
			// FMacApplication::ProcessScrollWheelEvent converts scrolls into gestures on Mac.
			// Convert it back to allow scrolling.
			SlateApp.ProcessMouseWheelOrGestureEvent(InWheelEvent, nullptr);
		}
		return true;
	}
	if (UINavPC != nullptr)
	{
		if (UINavPC->IsListeningToInputRebind())
		{
			const FKeyEvent MouseKeyEvent(InWheelEvent.GetWheelDelta() > 0.f ? EKeys::MouseScrollUp : EKeys::MouseScrollDown, FModifierKeysState(), InWheelEvent.GetUserIndex(), InWheelEvent.IsRepeat(), 0, 0);
			UINavPC->ProcessRebind(MouseKeyEvent);
			return true;
		}
		UINavPC->HandleMouseWheelOrGestureEvent(SlateApp, InWheelEvent, InGesture);
	}

	return IInputProcessor::HandleMouseWheelOrGestureEvent(SlateApp, InWheelEvent, InGesture);
}