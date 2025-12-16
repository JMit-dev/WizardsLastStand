// Actor that spawns the death screen UI when placed in the DeathScreen level.

#include "DeathScreenManager.h"
#include "DeathScreenWidget.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"

ADeathScreenManager::ADeathScreenManager()
{
	PrimaryActorTick.bCanEverTick = false;

	// Simple root so the actor can be placed in the level
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	SpawnedWidget = nullptr;
}

void ADeathScreenManager::BeginPlay()
{
	Super::BeginPlay();

	if (IsRunningDedicatedServer())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		FString MapName = World->GetMapName();
		MapName.RemoveFromStart(World->StreamingLevelsPrefix);

		// Only run on the DeathScreen map
		if (!MapName.Contains(TEXT("DeathScreen")))
		{
			return;
		}

		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			if (PC->IsLocalController())
			{
				// If a view target is set (or can be found), force the camera to it so designers can control the shot
				AActor* Target = ViewTargetActor;
				if (!Target)
				{
					// Look for an actor tagged "DeathCamera" or the first camera actor as a fallback
					for (TActorIterator<AActor> It(World); It; ++It)
					{
						if (It->ActorHasTag(FName("DeathCamera")))
						{
							Target = *It;
							break;
						}
					}

					if (!Target)
					{
						for (TActorIterator<ACameraActor> CamIt(World); CamIt; ++CamIt)
						{
							Target = *CamIt;
							break;
						}
					}
				}

				if (Target)
				{
					PC->SetViewTarget(Target);
				}

				SpawnedWidget = CreateWidget<UDeathScreenWidget>(PC, UDeathScreenWidget::StaticClass());
				if (SpawnedWidget)
				{
					SpawnedWidget->AddToViewport(100);
					UE_LOG(LogTemp, Log, TEXT("DeathScreenManager: Death UI added to viewport."));

					FInputModeUIOnly InputMode;
					InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
					InputMode.SetWidgetToFocus(SpawnedWidget->TakeWidget());
					PC->SetInputMode(InputMode);
					PC->bShowMouseCursor = true;
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("DeathScreenManager: Failed to create death UI widget."));
				}
			}
		}
	}
}
