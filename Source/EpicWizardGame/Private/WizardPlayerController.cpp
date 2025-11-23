// Fill out your copyright notice in the Description page of Project Settings.

#include "WizardPlayerController.h"
#include "HotbarWidget.h"

AWizardPlayerController::AWizardPlayerController()
{
}

void AWizardPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Set input mode to game only
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

void AWizardPlayerController::SetHotbarWidget(UHotbarWidget* Widget)
{
	HotbarWidget = Widget;
}
