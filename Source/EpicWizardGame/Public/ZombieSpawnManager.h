// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieSpawnManager.generated.h"

class AZombieSpawnGate;
class AZombieCharacter;

UCLASS()
class EPICWIZARDGAME_API AZombieSpawnManager : public AActor
{
	GENERATED_BODY()

protected:

	/** Maximum total number of zombies allowed at once across all gates */
	UPROPERTY(EditAnywhere, Category="Spawning")
	int32 MaxTotalZombies = 20;

	/** Time between spawn attempts (seconds) */
	UPROPERTY(EditAnywhere, Category="Spawning")
	float SpawnInterval = 2.0f;

	/** Auto-start spawning on begin play */
	UPROPERTY(EditAnywhere, Category="Spawning")
	bool bAutoStartSpawning = true;

	/** Array of all spawn gates in the level */
	UPROPERTY()
	TArray<AZombieSpawnGate*> SpawnGates;

	/** Array of all active zombies */
	UPROPERTY()
	TArray<AZombieCharacter*> ActiveZombies;

	/** Timer for spawn intervals */
	FTimerHandle SpawnTimer;

	/** Is spawning active */
	bool bIsSpawning = false;

public:

	// Sets default values for this actor's properties
	AZombieSpawnManager();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Start spawning zombies */
	UFUNCTION(BlueprintCallable, Category="Spawning")
	void StartSpawning();

	/** Stop spawning zombies */
	UFUNCTION(BlueprintCallable, Category="Spawning")
	void StopSpawning();

	/** Get current total zombie count */
	UFUNCTION(BlueprintCallable, Category="Spawning")
	int32 GetTotalZombieCount() const { return ActiveZombies.Num(); }

	/** Get max total zombies allowed */
	UFUNCTION(BlueprintCallable, Category="Spawning")
	int32 GetMaxTotalZombies() const { return MaxTotalZombies; }

protected:

	/** Find all spawn gates in the level */
	void FindSpawnGates();

	/** Called on timer to attempt spawning */
	void TrySpawnZombie();

	/** Called when a zombie dies */
	UFUNCTION()
	void OnZombieDied();

	/** Clean up null references from active zombies */
	void CleanupDeadZombies();
};
