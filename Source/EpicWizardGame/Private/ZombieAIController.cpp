// Fill out your copyright notice in the Description page of Project Settings.

#include "ZombieAIController.h"
#include "ZombieCharacter.h"
#include "Tower.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
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
	AActor* BestTarget = nullptr;
	float BestTargetScore = -FLT_MAX;

	// Find the best target based on path distance and reachability
	// Score is calculated as: 1000 / (PathDistance + 1) - higher score = better target

	// Check player
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		// Get PATH distance (not straight line distance)
		float PathDistance = -1.0f;
		FPathFindingQuery Query;
		if (UPathFollowingComponent* PathComp = GetPathFollowingComponent())
		{
			// Check if we can path to the player
			UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
			if (NavSys)
			{
				FPathFindingQuery PathQuery(this, *NavSys->GetDefaultNavDataInstance(), ZombieLocation, PlayerPawn->GetActorLocation());
				FPathFindingResult Result = NavSys->FindPathSync(PathQuery);

				if (Result.IsSuccessful() && Result.Path.IsValid())
				{
					PathDistance = Result.Path->GetLength();
				}
			}
		}

		// If we can path to player, score it
		if (PathDistance > 0.0f)
		{
			float Score = 1000.0f / (PathDistance + 1.0f);
			if (Score > BestTargetScore)
			{
				BestTargetScore = Score;
				BestTarget = PlayerPawn;
			}
		}
	}

	// Check all towers - prioritize towers heavily if player is unreachable
	for (TActorIterator<ATower> It(GetWorld()); It; ++It)
	{
		ATower* Tower = *It;
		if (Tower && !Tower->IsDestroyed())
		{
			// Get PATH distance to tower
			float PathDistance = -1.0f;
			if (UPathFollowingComponent* PathComp = GetPathFollowingComponent())
			{
				UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
				if (NavSys)
				{
					FPathFindingQuery PathQuery(this, *NavSys->GetDefaultNavDataInstance(), ZombieLocation, Tower->GetActorLocation());
					FPathFindingResult Result = NavSys->FindPathSync(PathQuery);

					if (Result.IsSuccessful() && Result.Path.IsValid())
					{
						PathDistance = Result.Path->GetLength();
					}
				}
			}

			// If we can path to tower, score it (towers get bonus priority)
			if (PathDistance > 0.0f)
			{
				// Give towers a 2x multiplier to prefer them when equally accessible
				float Score = 2000.0f / (PathDistance + 1.0f);
				if (Score > BestTargetScore)
				{
					BestTargetScore = Score;
					BestTarget = Tower;
				}
			}
		}
	}

	// Fallback: if no pathable target, just use nearest by straight distance
	if (!BestTarget)
	{
		float NearestDistance = FLT_MAX;

		if (PlayerPawn)
		{
			float PlayerDistance = FVector::Dist(ZombieLocation, PlayerPawn->GetActorLocation());
			if (PlayerDistance < NearestDistance)
			{
				NearestDistance = PlayerDistance;
				BestTarget = PlayerPawn;
			}
		}

		for (TActorIterator<ATower> It(GetWorld()); It; ++It)
		{
			ATower* Tower = *It;
			if (Tower && !Tower->IsDestroyed())
			{
				float TowerDistance = FVector::Dist(ZombieLocation, Tower->GetActorLocation());
				if (TowerDistance < NearestDistance)
				{
					NearestDistance = TowerDistance;
					BestTarget = Tower;
				}
			}
		}
	}

	// If no valid target found, do nothing
	if (!BestTarget)
	{
		return;
	}

	// Check if in attack range
	float DistanceToTarget = FVector::Dist(ZombieLocation, BestTarget->GetActorLocation());
	bool bInAttackRange = false;
	ATower* TargetTower = Cast<ATower>(BestTarget);

	if (TargetTower)
	{
		// For towers, also check overlap (in case tower is vertically stretched)
		TArray<AActor*> OverlappingActors;
		ZombieCharacter->GetOverlappingActors(OverlappingActors, ATower::StaticClass());
		bool bIsOverlapping = OverlappingActors.Contains(TargetTower);

		// In range if either overlapping OR within distance
		bInAttackRange = bIsOverlapping || (DistanceToTarget <= 300.0f);
	}
	else
	{
		// Use normal attack distance for player
		bInAttackRange = (DistanceToTarget <= AttackDistance);
	}

	if (bInAttackRange)
	{
		// Stop moving
		StopMovement();

		// Face the target
		FVector Direction = BestTarget->GetActorLocation() - ZombieLocation;
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
		// Move towards best target
		MoveToActor(BestTarget, AcceptanceRadius);
	}
}

void AZombieAIController::OnZombieDeath()
{
	// Stop AI updates
	GetWorld()->GetTimerManager().ClearTimer(AIUpdateTimer);

	// Stop movement
	StopMovement();
}
