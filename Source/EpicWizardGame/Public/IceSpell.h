// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "IceSpell.generated.h"

class ASpellProjectile;

/**
 * Ice spell - cone-shaped freeze effect
 */
UCLASS()
class EPICWIZARDGAME_API AIceSpell : public ASpellBase
{
	GENERATED_BODY()

public:

	AIceSpell();

	/** Cone range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float ConeRange = 500.0f;

	/** Cone angle in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float ConeAngle = 45.0f;

	/** Freeze duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float FreezeDuration = 3.0f;

	/** Movement speed multiplier when frozen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float FreezeSpeedMultiplier = 0.2f;

	/** Projectile class to spawn for the ice shard visual */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	TSubclassOf<ASpellProjectile> ProjectileClass;

	/** Spawn offset from character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float SpawnDistance = 100.0f;

	virtual void Execute(AWizardCharacter* Caster) override;
};
