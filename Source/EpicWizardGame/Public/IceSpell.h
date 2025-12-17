// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "IceSpell.generated.h"

class ASpellProjectile;

/**
 * Ice spell - projectile that freezes/slows on hit
 */
UCLASS()
class EPICWIZARDGAME_API AIceSpell : public ASpellBase
{
	GENERATED_BODY()

public:

	AIceSpell();

	/** Freeze duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float FreezeDuration = 3.0f;

	/** Movement speed multiplier when frozen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float FreezeSpeedMultiplier = 0.2f;

	/** Projectile class to spawn (should be a SpellProjectile with freeze enabled) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	TSubclassOf<ASpellProjectile> ProjectileClass;

	/** Spawn offset from character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float SpawnDistance = 100.0f;

	virtual void Execute(AWizardCharacter* Caster) override;
};
