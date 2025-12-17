// Fill out your copyright notice in the Description page of Project Settings.

#include "WaveManager.h"
#include "ZombieSpawnManager.h"
#include "BuildModeTimerWidget.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

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

	// Check if we're in a menu screen - if so, don't start waves
	FString CurrentLevelName = GetWorld()->GetMapName();
	CurrentLevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	if (CurrentLevelName.Contains(TEXT("TitleScreen")) || CurrentLevelName.Contains(TEXT("DeathScreen")))
	{
		UE_LOG(LogTemp, Log, TEXT("WaveManager: In TitleScreen/DeathScreen level, waves disabled"));
		return;
	}

	// Initialize money
	PlayerMoney = StartingMoney;
	UE_LOG(LogTemp, Log, TEXT("WaveManager: Starting money: $%d"), PlayerMoney);

	// Create and add build mode timer widget to viewport
	if (BuildModeTimerWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			BuildModeTimerWidget = CreateWidget<UBuildModeTimerWidget>(PC, BuildModeTimerWidgetClass);
			if (BuildModeTimerWidget)
			{
				BuildModeTimerWidget->AddToViewport(100); // High Z-order to overlay on top
				UE_LOG(LogTemp, Log, TEXT("WaveManager: Build mode timer widget created"));
			}
		}
	}

	// Find spawn manager
	FindSpawnManager();

	// Auto-start first wave if enabled (start wave 1 immediately, no build mode)
	if (bAutoStartFirstWave && SpawnManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaveManager: Starting Wave 1 immediately"));
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

	// Set wave as active and end build mode
	bWaveActive = true;
	bInBuildMode = false;

	// Update spawn manager with total zombies to spawn and health
	SpawnManager->TotalZombiesToSpawn = TotalZombiesThisWave;
	SpawnManager->ZombieHealthOverride = CalculateZombieHealth(CurrentWave);

	// Start spawning
	SpawnManager->StartSpawning();

	float ZombieHP = CalculateZombieHealth(CurrentWave);
	UE_LOG(LogTemp, Warning, TEXT("WaveManager: Round %d started! Zombies: %d | Zombie HP: %.0f | BUILD MODE ENDED"),
		CurrentWave, TotalZombiesThisWave, ZombieHP);
}

void AWaveManager::OnZombieDied()
{
	if (bWaveActive)
	{
		ZombiesKilledThisWave++;

		// Award money for kill
		int32 MoneyReward = CalculateMoneyReward();
		AddMoney(MoneyReward);

		UE_LOG(LogTemp, Log, TEXT("WaveManager: Zombie killed. %d/%d | +$%d (Total: $%d)"),
			ZombiesKilledThisWave, TotalZombiesThisWave, MoneyReward, PlayerMoney);
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

	// Restore player health at end of round (start of break)
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		// Get max health (assuming 100 for 3-hit system with 33.33 damage per hit)
		float MaxHealth = 100.0f;

		// Apply healing to restore to full
		UGameplayStatics::ApplyDamage(PlayerPawn, -MaxHealth, nullptr, nullptr, nullptr);
		UE_LOG(LogTemp, Log, TEXT("WaveManager: Player health restored"));
	}

	UE_LOG(LogTemp, Warning, TEXT("WaveManager: Round %d complete! Starting break..."), CurrentWave);

	// Start wave break
	StartWaveBreak();
}

void AWaveManager::StartWaveBreak()
{
	// Enter build mode
	bInBuildMode = true;
	BuildModeStartTime = GetWorld()->GetTimeSeconds();

	// Recreate timer widget if it was destroyed
	if (!BuildModeTimerWidget && BuildModeTimerWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("WaveManager: Timer widget was destroyed! Recreating..."));
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			BuildModeTimerWidget = CreateWidget<UBuildModeTimerWidget>(PC, BuildModeTimerWidgetClass);
			if (BuildModeTimerWidget)
			{
				BuildModeTimerWidget->AddToViewport(100);
				UE_LOG(LogTemp, Warning, TEXT("WaveManager: Recreated build mode timer widget"));
			}
		}
	}

	// Start timer for next wave
	GetWorld()->GetTimerManager().SetTimer(WaveBreakTimer, this, &AWaveManager::StartNextWave, TimeBetweenWaves, false);

	UE_LOG(LogTemp, Warning, TEXT("WaveManager: BUILD MODE STARTED - %.0f seconds until Round %d"), TimeBetweenWaves, CurrentWave + 1);
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

int32 AWaveManager::CalculateMoneyReward() const
{
	if (CurrentWave <= 0)
	{
		return BaseMoneyPerKill;
	}

	// Rounds 1-10: Static money reward
	if (CurrentWave <= 10)
	{
		return BaseMoneyPerKill;
	}

	// Round 11+: Money increases by 15% per round
	int32 RoundsAfter10 = CurrentWave - 10;
	float MoneyReward = BaseMoneyPerKill * FMath::Pow(MoneyMultiplierAfterRound10, RoundsAfter10);

	return FMath::RoundToInt(MoneyReward);
}

void AWaveManager::AddMoney(int32 Amount)
{
	PlayerMoney += Amount;
}

bool AWaveManager::SpendMoney(int32 Amount)
{
	if (PlayerMoney >= Amount)
	{
		PlayerMoney -= Amount;
		UE_LOG(LogTemp, Log, TEXT("WaveManager: Spent $%d. Remaining: $%d"), Amount, PlayerMoney);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("WaveManager: Not enough money! Need $%d, have $%d"), Amount, PlayerMoney);
	return false;
}

float AWaveManager::GetBuildModeTimeRemaining() const
{
	if (!bInBuildMode)
	{
		return 0.0f;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float ElapsedTime = CurrentTime - BuildModeStartTime;
	float RemainingTime = TimeBetweenWaves - ElapsedTime;

	return FMath::Max(0.0f, RemainingTime);
}

