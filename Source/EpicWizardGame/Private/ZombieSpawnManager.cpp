// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpawnManager.h"
#include "ZombieSpawnGate.h"
#include "ZombieCharacter.h"
#include "WaveManager.h"
#include "EngineUtils.h"
#include "TimerManager.h"

// Sets default values
AZombieSpawnManager::AZombieSpawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AZombieSpawnManager::BeginPlay()
{
	Super::BeginPlay();

	// Check if we're in a menu screen - if so, don't start spawning
	FString CurrentLevelName = GetWorld()->GetMapName();
	CurrentLevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	if (CurrentLevelName.Contains(TEXT("TitleScreen")) || CurrentLevelName.Contains(TEXT("DeathScreen")))
	{
		UE_LOG(LogTemp, Log, TEXT("ZombieSpawnManager: In TitleScreen/DeathScreen level, spawning disabled"));
		return;
	}

	// Find all spawn gates in the level
	FindSpawnGates();

	// Try to find wave manager
	for (TActorIterator<AWaveManager> It(GetWorld()); It; ++It)
	{
		WaveManager = *It;
		UE_LOG(LogTemp, Log, TEXT("ZombieSpawnManager: Found wave manager, will wait for it to control spawning"));
		break;
	}

	// Auto-start spawning if enabled and no wave manager
	if (bAutoStartSpawning && !WaveManager)
	{
		StartSpawning();
	}
}

// Called every frame
void AZombieSpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZombieSpawnManager::StartSpawning()
{
	if (bIsSpawning)
	{
		return;
	}

	bIsSpawning = true;
	TotalZombiesSpawned = 0; // Reset spawn counter for new wave

	// Start the spawn timer
	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AZombieSpawnManager::TrySpawnZombie, SpawnInterval, true);
}

void AZombieSpawnManager::StopSpawning()
{
	if (!bIsSpawning)
	{
		return;
	}

	bIsSpawning = false;

	// Clear the spawn timer
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
}

void AZombieSpawnManager::FindSpawnGates()
{
	SpawnGates.Empty();

	// Find all ZombieSpawnGate actors in the level
	for (TActorIterator<AZombieSpawnGate> It(GetWorld()); It; ++It)
	{
		SpawnGates.Add(*It);
	}

	UE_LOG(LogTemp, Log, TEXT("ZombieSpawnManager: Found %d spawn gates"), SpawnGates.Num());
}

void AZombieSpawnManager::TrySpawnZombie()
{
	// Clean up dead zombies first
	CleanupDeadZombies();

	// Check if we've spawned all zombies for this wave
	if (TotalZombiesToSpawn > 0 && TotalZombiesSpawned >= TotalZombiesToSpawn)
	{
		return;
	}

	// Check if we're at max alive capacity
	if (ActiveZombies.Num() >= MaxTotalZombies)
	{
		return;
	}

	// Check if we have any gates
	if (SpawnGates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZombieSpawnManager: No spawn gates found!"));
		return;
	}

	// Try to find a gate that can spawn
	TArray<AZombieSpawnGate*> AvailableGates;
	for (AZombieSpawnGate* Gate : SpawnGates)
	{
		if (Gate && Gate->CanSpawn())
		{
			AvailableGates.Add(Gate);
		}
	}

	// If no gates available, return
	if (AvailableGates.Num() == 0)
	{
		return;
	}

	// Pick a random available gate
	int32 RandomIndex = FMath::RandRange(0, AvailableGates.Num() - 1);
	AZombieSpawnGate* SelectedGate = AvailableGates[RandomIndex];

	// Spawn zombie from the selected gate
	AZombieCharacter* NewZombie = SelectedGate->SpawnZombie();

	if (NewZombie)
	{
		// Apply health override if set by wave manager
		if (ZombieHealthOverride > 0.0f)
		{
			NewZombie->MaxHP = ZombieHealthOverride;
			NewZombie->CurrentHP = ZombieHealthOverride;
		}

		// Add to active zombies
		ActiveZombies.Add(NewZombie);

		// Increment spawned counter
		TotalZombiesSpawned++;

		// Bind to death event
		NewZombie->OnZombieDeath.AddDynamic(this, &AZombieSpawnManager::OnZombieDied);

		UE_LOG(LogTemp, Log, TEXT("ZombieSpawnManager: Spawned zombie %d/%d (HP: %.0f). Alive: %d/%d"),
			TotalZombiesSpawned, TotalZombiesToSpawn, NewZombie->CurrentHP, ActiveZombies.Num(), MaxTotalZombies);
	}
}

void AZombieSpawnManager::OnZombieDied()
{
	// Notify wave manager if it exists
	if (WaveManager)
	{
		WaveManager->OnZombieDied();
	}

	// Clean up dead zombies
	CleanupDeadZombies();
}

void AZombieSpawnManager::CleanupDeadZombies()
{
	// Remove null or dead zombies from the array
	ActiveZombies.RemoveAll([](AZombieCharacter* Zombie)
	{
		return Zombie == nullptr || !IsValid(Zombie) || Zombie->IsDead();
	});
}

