// Fill out your copyright notice in the Description page of Project Settings.

#include "WizardPlayerController.h"
#include "HotbarWidget.h"

AWizardPlayerController::AWizardPlayerController()
{
}

void AWizardPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Keep cursor visible for top-down aiming without needing a "click to focus"
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AWizardPlayerController::SetHotbarWidget(UHotbarWidget* Widget)
{
	HotbarWidget = Widget;
}
