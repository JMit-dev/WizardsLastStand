// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpellBase.h"
#include "AirblastSpell.generated.h"

/**
 * Airblast spell - knocks back zombies in AOE with minimal damage
 */
UCLASS()
class EPICWIZARDGAME_API AAirblastSpell : public ASpellBase
{
	GENERATED_BODY()

public:

	AAirblastSpell();

	/** AOE radius around caster */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float BlastRadius = 400.0f;

	/** Knockback force strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float KnockbackForce = 1000.0f;

	/** Projectile class to spawn for the airblast */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	TSubclassOf<AActor> ProjectileClass;

	/** Projectile speed for airblast rectangle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float ProjectileSpeed = 1500.0f;

	/** How long the airblast projectile lasts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float ProjectileLifetime = 1.0f;

	virtual void Execute(AWizardCharacter* Caster) override;
};
