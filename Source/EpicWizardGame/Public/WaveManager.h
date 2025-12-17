// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveManager.generated.h"

class AZombieSpawnManager;
class UBuildModeTimerWidget;

UCLASS()
class EPICWIZARDGAME_API AWaveManager : public AActor
{
	GENERATED_BODY()

protected:

	/** Build mode timer widget class */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UBuildModeTimerWidget> BuildModeTimerWidgetClass;

	/** Instance of the build mode timer widget */
	UPROPERTY()
	UBuildModeTimerWidget* BuildModeTimerWidget;

	/** Current wave number */
	UPROPERTY(BlueprintReadOnly, Category="Wave System")
	int32 CurrentWave = 0;

	/** Player's current money */
	UPROPERTY(BlueprintReadOnly, Category="Economy")
	int32 PlayerMoney = 0;

	/** Base number of zombies for wave 1 */
	UPROPERTY(EditAnywhere, Category="Wave System")
	int32 BaseZombiesPerWave = 5;

	/** How many additional zombies per wave (approx 5-6 more per round after round 5) */
	UPROPERTY(EditAnywhere, Category="Wave System")
	float ZombiesIncreaseMultiplier = 0.15f;

	/** Time between waves (seconds) - BUILD MODE duration */
	UPROPERTY(EditAnywhere, Category="Wave System")
	float TimeBetweenWaves = 15.0f;

	/** Base zombie health for round 1 */
	UPROPERTY(EditAnywhere, Category="Wave System|Health")
	float BaseZombieHealth = 150.0f;

	/** Health increase per round for rounds 1-10 (linear) */
	UPROPERTY(EditAnywhere, Category="Wave System|Health")
	float HealthIncreasePerRound = 100.0f;

	/** Health multiplier per round after round 10 (1.1 = 10% increase) */
	UPROPERTY(EditAnywhere, Category="Wave System|Health")
	float HealthMultiplierAfterRound10 = 1.1f;

	/** Base money reward per zombie kill (rounds 1-10) */
	UPROPERTY(EditAnywhere, Category="Economy")
	int32 BaseMoneyPerKill = 60;

	/** Money multiplier per round after round 10 */
	UPROPERTY(EditAnywhere, Category="Economy")
	float MoneyMultiplierAfterRound10 = 1.15f;

	/** Starting money for the player */
	UPROPERTY(EditAnywhere, Category="Economy")
	int32 StartingMoney = 500;

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

	/** Is currently in build mode (wave break period) */
	bool bInBuildMode = false;

	/** Timer for wave break period */
	FTimerHandle WaveBreakTimer;

	/** Time when build mode started */
	float BuildModeStartTime = 0.0f;

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

	/** Is currently in build mode (can place turrets) */
	UFUNCTION(BlueprintPure, Category="Wave System")
	bool IsInBuildMode() const { return bInBuildMode; }

	/** Get remaining time in build mode (seconds) */
	UFUNCTION(BlueprintPure, Category="Wave System")
	float GetBuildModeTimeRemaining() const;

	/** Calculate zombie health for a given round */
	UFUNCTION(BlueprintPure, Category="Wave System")
	float CalculateZombieHealth(int32 RoundNumber) const;

	/** Calculate zombie count for a given round */
	UFUNCTION(BlueprintPure, Category="Wave System")
	int32 CalculateZombieCount(int32 RoundNumber) const;

	/** Calculate money reward for killing a zombie in current round */
	UFUNCTION(BlueprintPure, Category="Economy")
	int32 CalculateMoneyReward() const;

	/** Add money to player */
	UFUNCTION(BlueprintCallable, Category="Economy")
	void AddMoney(int32 Amount);

	/** Subtract money from player (returns true if successful) */
	UFUNCTION(BlueprintCallable, Category="Economy")
	bool SpendMoney(int32 Amount);

	/** Get current player money */
	UFUNCTION(BlueprintPure, Category="Economy")
	int32 GetPlayerMoney() const { return PlayerMoney; }

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
