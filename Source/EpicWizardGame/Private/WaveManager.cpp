// Fill out your copyright notice in the Description page of Project Settings.


#include "WaveManager.h"
#include "ZombieSpawnManager.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AWaveManager::AWaveManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWaveManager::BeginPlay()
{
	Super::BeginPlay();

	// Check if we're in the title screen - if so, don't start waves
	FString CurrentLevelName = GetWorld()->GetMapName();
	CurrentLevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	if (CurrentLevelName.Contains(TEXT("TitleScreen")))
	{
		UE_LOG(LogTemp, Log, TEXT("WaveManager: In TitleScreen level, waves disabled"));
		return;
	}

	// Find spawn manager
	FindSpawnManager();

	// Auto-start first wave if enabled
	if (bAutoStartFirstWave && SpawnManager)
	{
		StartNextWave();
	}
}

// Called every frame
void AWaveManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if wave is complete during active wave
	if (bWaveActive)
	{
		CheckWaveComplete();
	}
}

void AWaveManager::StartNextWave()
{
	if (!SpawnManager)
	{
		UE_LOG(LogTemp, Error, TEXT("WaveManager: No spawn manager found!"));
		return;
	}

	// Increment wave number
	CurrentWave++;

	// Calculate zombies for this wave using COD formula
	TotalZombiesThisWave = CalculateZombieCount(CurrentWave);
	ZombiesKilledThisWave = 0;

	// Set wave as active
	bWaveActive = true;

	// Update spawn manager with total zombies to spawn and health
	SpawnManager->TotalZombiesToSpawn = TotalZombiesThisWave;
	SpawnManager->ZombieHealthOverride = CalculateZombieHealth(CurrentWave);

	// Restore player health at start of each round
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		// Get max health (assuming 100 for 3-hit system with 33.33 damage per hit)
		float MaxHealth = 100.0f;

		// Apply healing to restore to full
		UGameplayStatics::ApplyDamage(PlayerPawn, -MaxHealth, nullptr, nullptr, nullptr);
		UE_LOG(LogTemp, Log, TEXT("WaveManager: Player health restored"));
	}

	// Start spawning
	SpawnManager->StartSpawning();

	float ZombieHP = CalculateZombieHealth(CurrentWave);
	UE_LOG(LogTemp, Warning, TEXT("WaveManager: Round %d started! Zombies: %d | Zombie HP: %.0f"),
		CurrentWave, TotalZombiesThisWave, ZombieHP);
}

void AWaveManager::OnZombieDied()
{
	if (bWaveActive)
	{
		ZombiesKilledThisWave++;
		UE_LOG(LogTemp, Log, TEXT("WaveManager: Zombie killed. %d/%d"), ZombiesKilledThisWave, TotalZombiesThisWave);
	}
}

void AWaveManager::FindSpawnManager()
{
	// Find ZombieSpawnManager in the level
	for (TActorIterator<AZombieSpawnManager> It(GetWorld()); It; ++It)
	{
		SpawnManager = *It;
		UE_LOG(LogTemp, Log, TEXT("WaveManager: Found spawn manager"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("WaveManager: No spawn manager found in level!"));
}

void AWaveManager::CheckWaveComplete()
{
	if (!SpawnManager)
	{
		return;
	}

	// Wave is complete when all zombies are killed
	if (ZombiesKilledThisWave >= TotalZombiesThisWave && SpawnManager->GetTotalZombieCount() == 0)
	{
		OnWaveComplete();
	}
}

void AWaveManager::OnWaveComplete()
{
	bWaveActive = false;

	// Stop spawning
	if (SpawnManager)
	{
		SpawnManager->StopSpawning();
	}

	UE_LOG(LogTemp, Warning, TEXT("WaveManager: Wave %d complete! Starting break..."), CurrentWave);

	// Start wave break
	StartWaveBreak();
}

void AWaveManager::StartWaveBreak()
{
	// Start timer for next wave
	GetWorld()->GetTimerManager().SetTimer(WaveBreakTimer, this, &AWaveManager::StartNextWave, TimeBetweenWaves, false);

	UE_LOG(LogTemp, Warning, TEXT("WaveManager: %.0f second break before Round %d"), TimeBetweenWaves, CurrentWave + 1);
}

float AWaveManager::CalculateZombieHealth(int32 RoundNumber) const
{
	if (RoundNumber <= 0)
	{
		return BaseZombieHealth;
	}

	// Rounds 1-10: Linear scaling
	if (RoundNumber <= 10)
	{
		return BaseZombieHealth + ((RoundNumber - 1) * HealthIncreasePerRound);
	}

	// Round 11+: Start with round 10 health, then multiply by 10% each round
	float Round10Health = BaseZombieHealth + (9 * HealthIncreasePerRound);
	int32 RoundsAfter10 = RoundNumber - 10;

	// Apply 10% compounding increase for each round after 10
	float FinalHealth = Round10Health * FMath::Pow(HealthMultiplierAfterRound10, RoundsAfter10);

	return FinalHealth;
}

int32 AWaveManager::CalculateZombieCount(int32 RoundNumber) const
{
	if (RoundNumber <= 0)
	{
		return BaseZombiesPerWave;
	}

	// COD Zombies formula approximation:
	// Round 1: 6 zombies (we use 5)
	// Increases by ~15-20% per round, with some variation
	// Formula: Base * (1 + Multiplier)^(Round - 1)
	// This gives roughly: R1=5, R2=6, R3=7, R4=8, R5=9, R10=20, R20=82, R30=331

	float ZombieCount = BaseZombiesPerWave * FMath::Pow(1.0f + ZombiesIncreaseMultiplier, RoundNumber - 1);

	// Round to nearest integer
	return FMath::RoundToInt(ZombieCount);
}

