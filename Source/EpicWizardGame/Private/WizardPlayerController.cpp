// Fill out your copyright notice in the Description page of Project Settings.

#include "WizardPlayerController.h"
#include "HotbarWidget.h"
#include "DeathScreenWidget.h"
#include "Kismet/GameplayStatics.h"

AWizardPlayerController::AWizardPlayerController()
{
	DeathScreenWidget = nullptr;
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

	// If we're on the DeathScreen level, show a minimal death UI and route input to it
	if (IsLocalPlayerController())
	{
		FString CurrentLevelName = GetWorld()->GetMapName();
		CurrentLevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
		if (CurrentLevelName.Contains(TEXT("DeathScreen")))
		{
			DeathScreenWidget = CreateWidget<UDeathScreenWidget>(this, UDeathScreenWidget::StaticClass());
			if (DeathScreenWidget)
			{
				DeathScreenWidget->AddToViewport(100);

				FInputModeUIOnly DeathInputMode;
				DeathInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				DeathInputMode.SetWidgetToFocus(DeathScreenWidget->TakeWidget());
				SetInputMode(DeathInputMode);

				bShowMouseCursor = true;
			}
		}
	}
}

void AWizardPlayerController::SetHotbarWidget(UHotbarWidget* Widget)
{
	HotbarWidget = Widget;
}
