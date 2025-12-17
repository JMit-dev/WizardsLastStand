// Fill out your copyright notice in the Description page of Project Settings.

#include "FloorCameraWidget.h"
#include "FloorCameraManager.h"
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

void UFloorCameraWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Try to find the camera manager in the level
	FindCameraManager();
}

void UFloorCameraWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update the camera feed every frame
	UpdateCameraFeed();
}

void UFloorCameraWidget::UpdateCameraFeed()
{
	if (!CameraManager)
	{
		// Try to find it if we don't have it yet
		FindCameraManager();
		return;
	}

	if (!CameraImage)
	{
		return;
	}

	// Get the render target for the floor the player is NOT on
	UTextureRenderTarget2D* RenderTarget = CameraManager->GetActiveRenderTarget();

	if (RenderTarget)
	{
		// Set the image brush to use the render target
		// Use SetBrushResourceObject for render targets
		CameraImage->SetBrushResourceObject(RenderTarget);
	}
}

void UFloorCameraWidget::FindCameraManager()
{
	// Search for FloorCameraManager in the level
	for (TActorIterator<AFloorCameraManager> It(GetWorld()); It; ++It)
	{
		CameraManager = *It;
		UE_LOG(LogTemp, Log, TEXT("Floor Camera Widget: Found Camera Manager"));
		break;
	}
}

