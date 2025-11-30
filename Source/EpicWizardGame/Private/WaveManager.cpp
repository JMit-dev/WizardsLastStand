// Fill out your copyright notice in the Description page of Project Settings.


#include "WaveManager.h"
#include "ZombieSpawnManager.h"
#include "EngineUtils.h"
#include "TimerManager.h"

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

	// Calculate zombies for this wave
	TotalZombiesThisWave = BaseZombiesPerWave + ((CurrentWave - 1) * ZombiesIncreasePerWave);
	ZombiesKilledThisWave = 0;

	// Set wave as active
	bWaveActive = true;

	// Update spawn manager with new zombie limit
	SpawnManager->MaxTotalZombies = TotalZombiesThisWave;

	// Start spawning
	SpawnManager->StartSpawning();

	UE_LOG(LogTemp, Warning, TEXT("WaveManager: Wave %d started! Total zombies: %d"), CurrentWave, TotalZombiesThisWave);
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
}

