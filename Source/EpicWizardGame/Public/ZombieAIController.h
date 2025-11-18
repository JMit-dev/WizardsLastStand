// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZombieAIController.generated.h"

class AZombieCharacter;

/**
 * AI Controller for the Zombie enemy
 * Handles chasing and attacking the player
 */
UCLASS()
class EPICWIZARDGAME_API AZombieAIController : public AAIController
{
	GENERATED_BODY()

protected:

	/** Distance at which zombie will attack */
	UPROPERTY(EditAnywhere, Category="AI")
	float AttackDistance = 150.0f;

	/** How often to update AI (seconds) */
	UPROPERTY(EditAnywhere, Category="AI")
	float AIUpdateInterval = 0.25f;

	/** Acceptance radius for move to */
	UPROPERTY(EditAnywhere, Category="AI")
	float AcceptanceRadius = 50.0f;

	/** Timer for AI updates */
	FTimerHandle AIUpdateTimer;

	/** Cached reference to zombie character */
	TObjectPtr<AZombieCharacter> ZombieCharacter;

public:

	AZombieAIController();

protected:

	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;

	/** Called on timer to update AI behavior */
	void UpdateAI();

	/** Called when zombie dies */
	UFUNCTION()
	void OnZombieDeath();
};
