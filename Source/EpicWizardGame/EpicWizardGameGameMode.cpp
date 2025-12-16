// Copyright Epic Games, Inc. All Rights Reserved.

#include "EpicWizardGameGameMode.h"
#include "DeathScreenWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AEpicWizardGameGameMode::AEpicWizardGameGameMode()
{
	// stub
}

void AEpicWizardGameGameMode::StartPlay()
{
	Super::StartPlay();

	if (IsRunningDedicatedServer())
	{
		return;
	}

	// If we're on the DeathScreen map, ensure a death UI exists even if the controller class is different
	if (UWorld* World = GetWorld())
	{
		FString MapName = World->GetMapName();
		MapName.RemoveFromStart(World->StreamingLevelsPrefix);
		UE_LOG(LogTemp, Log, TEXT("EpicWizardGameGameMode::StartPlay MapName=%s"), *MapName);
		if (MapName.Contains(TEXT("DeathScreen")))
		{
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				if (APlayerController* PC = It->Get())
				{
					if (PC->IsLocalController())
					{
						if (UDeathScreenWidget* Widget = CreateWidget<UDeathScreenWidget>(PC, UDeathScreenWidget::StaticClass()))
						{
							Widget->AddToViewport(100);
							UE_LOG(LogTemp, Log, TEXT("DeathScreen widget added to viewport from GameMode."));

							FInputModeUIOnly InputMode;
							InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
							InputMode.SetWidgetToFocus(Widget->TakeWidget());
							PC->SetInputMode(InputMode);
							PC->bShowMouseCursor = true;
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("DeathScreen widget failed to create in GameMode."));
						}
					}
				}
			}
		}
	}
}
