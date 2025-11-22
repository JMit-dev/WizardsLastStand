// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpawnManager.h"
#include "ZombieSpawnGate.h"
#include "ZombieCharacter.h"
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

	// Find all spawn gates in the level
	FindSpawnGates();

	// Auto-start spawning if enabled
	if (bAutoStartSpawning)
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

	// Check if we're at max capacity
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
		// Add to active zombies
		ActiveZombies.Add(NewZombie);

		// Bind to death event
		NewZombie->OnZombieDeath.AddDynamic(this, &AZombieSpawnManager::OnZombieDied);

		UE_LOG(LogTemp, Log, TEXT("ZombieSpawnManager: Spawned zombie. Total: %d/%d"), ActiveZombies.Num(), MaxTotalZombies);
	}
}

void AZombieSpawnManager::OnZombieDied()
{
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

