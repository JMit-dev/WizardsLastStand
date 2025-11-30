// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "LightningSpell.generated.h"

class ASpellProjectile;

/**
 * Lightning spell - strikes target from above with AOE damage
 */
UCLASS()
class EPICWIZARDGAME_API ALightningSpell : public ASpellBase
{
	GENERATED_BODY()

public:

	ALightningSpell();

	/** Raycast range to find target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float RaycastRange = 2000.0f;

	/** AOE radius around strike point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float AOERadius = 300.0f;

	/** AOE damage multiplier (AOE targets take BaseDamage * this) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float AOEDamageMultiplier = 0.5f;

	/** Projectile class to spawn for lightning visual */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	TSubclassOf<ASpellProjectile> ProjectileClass;

	/** Spawn offset from character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float SpawnDistance = 100.0f;

	virtual void Execute(AWizardCharacter* Caster) override;
};
