// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorCameraManager.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class APortal;

/**
 * Manages floor cameras for the Ramparts level
 * Shows a mini-view of the floor the player is not currently on
 */
UCLASS()
class EPICWIZARDGAME_API AFloorCameraManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFloorCameraManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Scene capture component for floor 1 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USceneCaptureComponent2D* Floor1Camera;

	/** Scene capture component for floor 2 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USceneCaptureComponent2D* Floor2Camera;

	/** Render target for floor 1 camera (create in editor: RT_Floor1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cameras")
	UTextureRenderTarget2D* Floor1RenderTarget;

	/** Render target for floor 2 camera (create in editor: RT_Floor2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cameras")
	UTextureRenderTarget2D* Floor2RenderTarget;

	/** Location for floor 1 camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cameras")
	FVector Floor1CameraLocation = FVector(0, 0, 500);

	/** Rotation for floor 1 camera (top-down view) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cameras")
	FRotator Floor1CameraRotation = FRotator(-90, 0, 0);

	/** Location for floor 2 camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cameras")
	FVector Floor2CameraLocation = FVector(0, 0, 1000);

	/** Rotation for floor 2 camera (top-down view) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cameras")
	FRotator Floor2CameraRotation = FRotator(-90, 0, 0);

	/** Which floor is the player currently on? (1 or 2) */
	UPROPERTY(BlueprintReadOnly, Category="Floor Tracking")
	int32 CurrentFloor = 1;

	/** Height threshold to determine which floor player is on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floor Tracking")
	float FloorSeparationHeight = 500.0f;

	/** Base Z height of floor 1 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Floor Tracking")
	float Floor1Height = 0.0f;

	/** Returns the render target for the floor the player is NOT on */
	UFUNCTION(BlueprintCallable, Category="Floor Camera")
	UTextureRenderTarget2D* GetActiveRenderTarget() const;

	/** Determine which floor the player is currently on based on Z position */
	UFUNCTION(BlueprintCallable, Category="Floor Camera")
	void UpdatePlayerFloor();

	/** Force set which floor the player is on (called by portals) */
	UFUNCTION(BlueprintCallable, Category="Floor Camera")
	void SetPlayerFloor(int32 FloorNumber);

	/** Enable/disable the camera system */
	UFUNCTION(BlueprintCallable, Category="Floor Camera")
	void SetCameraSystemEnabled(bool bEnabled);
};
