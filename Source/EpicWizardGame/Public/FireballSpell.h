// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "FireballSpell.generated.h"

class ASpellProjectile;

/**
 * Fireball spell - shoots a red cube projectile
 */
UCLASS()
class EPICWIZARDGAME_API AFireballSpell : public ASpellBase
{
	GENERATED_BODY()

public:

	AFireballSpell();

	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	TSubclassOf<ASpellProjectile> ProjectileClass;

	/** Spawn offset from character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float SpawnDistance = 100.0f;

	virtual void Execute(AWizardCharacter* Caster) override;
};
