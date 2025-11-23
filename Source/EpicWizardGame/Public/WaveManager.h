// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveManager.generated.h"

class AZombieSpawnManager;

UCLASS()
class EPICWIZARDGAME_API AWaveManager : public AActor
{
	GENERATED_BODY()

protected:

	/** Current wave number */
	UPROPERTY(BlueprintReadOnly, Category="Wave System")
	int32 CurrentWave = 0;

	/** Base number of zombies for wave 1 */
	UPROPERTY(EditAnywhere, Category="Wave System")
	int32 BaseZombiesPerWave = 10;

	/** How many additional zombies per wave */
	UPROPERTY(EditAnywhere, Category="Wave System")
	int32 ZombiesIncreasePerWave = 5;

	/** Time between waves (seconds) */
	UPROPERTY(EditAnywhere, Category="Wave System")
	float TimeBetweenWaves = 10.0f;

	/** Auto-start first wave on begin play */
	UPROPERTY(EditAnywhere, Category="Wave System")
	bool bAutoStartFirstWave = true;

	/** Reference to spawn manager */
	UPROPERTY()
	AZombieSpawnManager* SpawnManager;

	/** Total zombies to spawn this wave */
	int32 TotalZombiesThisWave = 0;

	/** Zombies killed this wave */
	int32 ZombiesKilledThisWave = 0;

	/** Is wave currently active */
	bool bWaveActive = false;

	/** Timer for wave break period */
	FTimerHandle WaveBreakTimer;

public:

	// Sets default values for this actor's properties
	AWaveManager();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Start the next wave */
	UFUNCTION(BlueprintCallable, Category="Wave System")
	void StartNextWave();

	/** Get current wave number */
	UFUNCTION(BlueprintPure, Category="Wave System")
	int32 GetCurrentWave() const { return CurrentWave; }

	/** Get total zombies for current wave */
	UFUNCTION(BlueprintPure, Category="Wave System")
	int32 GetTotalZombiesThisWave() const { return TotalZombiesThisWave; }

	/** Get zombies killed this wave */
	UFUNCTION(BlueprintPure, Category="Wave System")
	int32 GetZombiesKilledThisWave() const { return ZombiesKilledThisWave; }

	/** Get zombies remaining this wave */
	UFUNCTION(BlueprintPure, Category="Wave System")
	int32 GetZombiesRemainingThisWave() const { return TotalZombiesThisWave - ZombiesKilledThisWave; }

	/** Is wave currently active */
	UFUNCTION(BlueprintPure, Category="Wave System")
	bool IsWaveActive() const { return bWaveActive; }

	/** Called when a zombie dies */
	UFUNCTION()
	void OnZombieDied();

protected:

	/** Find spawn manager in level */
	void FindSpawnManager();

	/** Check if wave is complete */
	void CheckWaveComplete();

	/** Called when wave is complete */
	void OnWaveComplete();

	/** Start wave break timer */
	void StartWaveBreak();
};
