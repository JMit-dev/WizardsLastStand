// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FloorCameraWidget.generated.h"

class AFloorCameraManager;
class UImage;
class UTextureRenderTarget2D;

/**
 * Widget that displays the mini-view of the other floor
 * Shows a picture-in-picture of the floor the player is not on
 */
UCLASS()
class EPICWIZARDGAME_API UFloorCameraWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Reference to the floor camera manager in the level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floor Camera")
	AFloorCameraManager* CameraManager;

	/** The image widget that will display the camera feed (bind in BP) */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImage* CameraImage;

	/** Update the camera feed */
	UFUNCTION(BlueprintCallable, Category="Floor Camera")
	void UpdateCameraFeed();

protected:
	/** Find the camera manager in the level */
	void FindCameraManager();
};
