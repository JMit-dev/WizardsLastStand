// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieSpawnGate.generated.h"

class AZombieCharacter;
class UBoxComponent;
class UArrowComponent;

UCLASS()
class EPICWIZARDGAME_API AZombieSpawnGate : public AActor
{
	GENERATED_BODY()

protected:

	/** Box component for spawn area */
	UPROPERTY(VisibleAnywhere, Category="Components")
	UBoxComponent* SpawnArea;

	/** Visual mesh for the gate (editor visualization) */
	UPROPERTY(VisibleAnywhere, Category="Components")
	UStaticMeshComponent* GateMesh;

	/** Arrow component showing spawn direction */
	UPROPERTY(VisibleAnywhere, Category="Components")
	UArrowComponent* SpawnDirection;

	/** Zombie class to spawn */
	UPROPERTY(EditAnywhere, Category="Spawning")
	TSubclassOf<AZombieCharacter> ZombieClass;

	/** Maximum number of zombies this gate can have alive at once */
	UPROPERTY(EditAnywhere, Category="Spawning", meta=(ClampMin="1"))
	int32 MaxActiveZombiesPerGate = 5;

	/** Currently active zombies spawned by this gate */
	UPROPERTY()
	TArray<AZombieCharacter*> ActiveZombies;

public:

	// Sets default values for this actor's properties
	AZombieSpawnGate();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Spawns a zombie at a random location within the spawn area */
	UFUNCTION(BlueprintCallable, Category="Spawning")
	AZombieCharacter* SpawnZombie();

	/** Returns true if this gate can spawn more zombies */
	UFUNCTION(BlueprintCallable, Category="Spawning")
	bool CanSpawn() const;

	/** Returns the number of active zombies from this gate */
	UFUNCTION(BlueprintCallable, Category="Spawning")
	int32 GetActiveZombieCount() const { return ActiveZombies.Num(); }

	/** Returns the maximum zombies allowed per gate */
	UFUNCTION(BlueprintCallable, Category="Spawning")
	int32 GetMaxActiveZombies() const { return MaxActiveZombiesPerGate; }

protected:

	/** Called when a spawned zombie dies */
	UFUNCTION()
	void OnZombieDied();

	/** Clean up null references from the active zombies array */
	void CleanupDeadZombies();

	/** Get a random spawn location within the box */
	FVector GetRandomSpawnLocation() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
