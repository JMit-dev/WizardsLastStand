// Fill out your copyright notice in the Description page of Project Settings.

#include "WizardPlayerController.h"

AWizardPlayerController::AWizardPlayerController()
{
}

void AWizardPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Set input mode to game only and hide cursor
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}
