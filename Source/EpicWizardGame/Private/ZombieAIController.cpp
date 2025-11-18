// Fill out your copyright notice in the Description page of Project Settings.

#include "ZombieAIController.h"
#include "ZombieCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"

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

	// Find player
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	// Get distance to player
	float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());

	// If in attack range, attack
	if (Distance <= AttackDistance)
	{
		// Stop moving
		StopMovement();

		// Face the player
		FVector Direction = PlayerPawn->GetActorLocation() - GetPawn()->GetActorLocation();
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
		// Move towards player
		MoveToActor(PlayerPawn, AcceptanceRadius);
	}
}

void AZombieAIController::OnZombieDeath()
{
	// Stop AI updates
	GetWorld()->GetTimerManager().ClearTimer(AIUpdateTimer);

	// Stop movement
	StopMovement();
}
