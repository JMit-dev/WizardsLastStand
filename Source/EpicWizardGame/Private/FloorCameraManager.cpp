// Fill out your copyright notice in the Description page of Project Settings.

#include "FloorCameraManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

// Sets default values
AFloorCameraManager::AFloorCameraManager()
{
	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create scene capture components for both floors
	Floor1Camera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Floor1Camera"));
	Floor1Camera->SetupAttachment(RootComponent);
	Floor1Camera->ProjectionType = ECameraProjectionMode::Perspective;
	Floor1Camera->FOVAngle = 90.0f;
	Floor1Camera->bCaptureEveryFrame = true;
	Floor1Camera->bCaptureOnMovement = false;

	Floor2Camera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Floor2Camera"));
	Floor2Camera->SetupAttachment(RootComponent);
	Floor2Camera->ProjectionType = ECameraProjectionMode::Perspective;
	Floor2Camera->FOVAngle = 90.0f;
	Floor2Camera->bCaptureEveryFrame = true;
	Floor2Camera->bCaptureOnMovement = false;
}

// Called when the game starts or when spawned
void AFloorCameraManager::BeginPlay()
{
	Super::BeginPlay();

	// Set camera positions
	Floor1Camera->SetRelativeLocation(Floor1CameraLocation);
	Floor1Camera->SetRelativeRotation(Floor1CameraRotation);

	Floor2Camera->SetRelativeLocation(Floor2CameraLocation);
	Floor2Camera->SetRelativeRotation(Floor2CameraRotation);

	// Assign render targets
	if (Floor1RenderTarget)
	{
		Floor1Camera->TextureTarget = Floor1RenderTarget;
	}

	if (Floor2RenderTarget)
	{
		Floor2Camera->TextureTarget = Floor2RenderTarget;
	}

	// Initial floor detection
	UpdatePlayerFloor();
}

// Called every frame
void AFloorCameraManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Continuously update which floor the player is on
	UpdatePlayerFloor();
}

UTextureRenderTarget2D* AFloorCameraManager::GetActiveRenderTarget() const
{
	// Return the render target of the floor the player is NOT on
	if (CurrentFloor == 1)
	{
		return Floor2RenderTarget; // Show floor 2
	}
	else
	{
		return Floor1RenderTarget; // Show floor 1
	}
}

void AFloorCameraManager::UpdatePlayerFloor()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		return;
	}

	// Get player Z position
	float PlayerZ = PlayerPawn->GetActorLocation().Z;

	// Determine floor based on height
	// If player is above the separation height, they're on floor 2
	if (PlayerZ >= (Floor1Height + FloorSeparationHeight))
	{
		if (CurrentFloor != 2)
		{
			CurrentFloor = 2;
			UE_LOG(LogTemp, Log, TEXT("Floor Camera: Player moved to Floor 2"));
		}
	}
	else
	{
		if (CurrentFloor != 1)
		{
			CurrentFloor = 1;
			UE_LOG(LogTemp, Log, TEXT("Floor Camera: Player moved to Floor 1"));
		}
	}
}

void AFloorCameraManager::SetPlayerFloor(int32 FloorNumber)
{
	if (FloorNumber == 1 || FloorNumber == 2)
	{
		CurrentFloor = FloorNumber;
		UE_LOG(LogTemp, Log, TEXT("Floor Camera: Manually set to Floor %d"), FloorNumber);
	}
}

void AFloorCameraManager::SetCameraSystemEnabled(bool bEnabled)
{
	if (Floor1Camera)
	{
		Floor1Camera->SetActive(bEnabled);
	}

	if (Floor2Camera)
	{
		Floor2Camera->SetActive(bEnabled);
	}
}

