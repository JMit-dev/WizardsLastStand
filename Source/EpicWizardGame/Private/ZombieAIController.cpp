// Fill out your copyright notice in the Description page of Project Settings.

#include "ZombieAIController.h"
#include "ZombieCharacter.h"
#include "Tower.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "EngineUtils.h"

AZombieAIController::AZombieAIController()
{
}

void AZombieAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Cache zombie character
	ZombieCharacter = Cast<AZombieCharacter>(InPawn);

	if (ZombieCharacter)
	{
		// Bind to death event
		ZombieCharacter->OnZombieDeath.AddDynamic(this, &AZombieAIController::OnZombieDeath);

		// Start AI update timer
		GetWorld()->GetTimerManager().SetTimer(AIUpdateTimer, this, &AZombieAIController::UpdateAI, AIUpdateInterval, true);
	}
}

void AZombieAIController::OnUnPossess()
{
	// Clear timer
	GetWorld()->GetTimerManager().ClearTimer(AIUpdateTimer);

	// Unbind death event
	if (ZombieCharacter)
	{
		ZombieCharacter->OnZombieDeath.RemoveDynamic(this, &AZombieAIController::OnZombieDeath);
		ZombieCharacter = nullptr;
	}

	Super::OnUnPossess();
}

void AZombieAIController::UpdateAI()
{
	if (!ZombieCharacter || ZombieCharacter->IsDead())
	{
		return;
	}

	FVector ZombieLocation = GetPawn()->GetActorLocation();
	AActor* NearestTarget = nullptr;
	float NearestDistance = FLT_MAX;

	// Check player
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		float PlayerDistance = FVector::Dist(ZombieLocation, PlayerPawn->GetActorLocation());
		if (PlayerDistance < NearestDistance)
		{
			NearestDistance = PlayerDistance;
			NearestTarget = PlayerPawn;
		}
	}

	// Check all towers
	for (TActorIterator<ATower> It(GetWorld()); It; ++It)
	{
		ATower* Tower = *It;
		if (Tower && !Tower->IsDestroyed())
		{
			float TowerDistance = FVector::Dist(ZombieLocation, Tower->GetActorLocation());
			if (TowerDistance < NearestDistance)
			{
				NearestDistance = TowerDistance;
				NearestTarget = Tower;
			}
		}
	}

	// If no valid target found, do nothing
	if (!NearestTarget)
	{
		return;
	}

	// Check if in attack range (use different ranges for player vs tower)
	bool bInAttackRange = false;
	ATower* TargetTower = Cast<ATower>(NearestTarget);

	if (TargetTower)
	{
		// Use larger attack distance for towers
		bInAttackRange = (NearestDistance <= 300.0f);
	}
	else
	{
		// Use normal attack distance for player
		bInAttackRange = (NearestDistance <= AttackDistance);
	}

	if (bInAttackRange)
	{
		// Stop moving
		StopMovement();

		// Face the target
		FVector Direction = NearestTarget->GetActorLocation() - ZombieLocation;
		Direction.Z = 0.0f;
		if (!Direction.IsNearlyZero())
		{
			GetPawn()->SetActorRotation(Direction.Rotation());
		}

		// Attack if not already attacking
		if (!ZombieCharacter->IsAttacking())
		{
			ZombieCharacter->DoAttack();
		}
	}
	else
	{
		// Move towards nearest target
		MoveToActor(NearestTarget, AcceptanceRadius);
	}
}

void AZombieAIController::OnZombieDeath()
{
	// Stop AI updates
	GetWorld()->GetTimerManager().ClearTimer(AIUpdateTimer);

	// Stop movement
	StopMovement();
}
