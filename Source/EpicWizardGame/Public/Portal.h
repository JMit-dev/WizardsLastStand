// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class UBoxComponent;
class UArrowComponent;

UCLASS()
class EPICWIZARDGAME_API APortal : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APortal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Trigger box for portal entry */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	/** Arrow showing entry/exit direction */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* DirectionArrow;

	/** The linked portal to teleport to */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Portal")
	APortal* LinkedPortal;

	/** Cooldown to prevent infinite teleport loops */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float TeleportCooldown = 1.0f;

	/** Actors currently on cooldown */
	TMap<AActor*, float> CooldownMap;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Called when something enters the portal trigger */
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Teleport an actor to the linked portal */
	void TeleportActor(AActor* ActorToTeleport);

	/** Check if actor is on cooldown */
	bool IsOnCooldown(AActor* Actor) const;

	/** Add actor to cooldown */
	void AddToCooldown(AActor* Actor);
};
