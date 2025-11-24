// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpellBase.generated.h"

class AWizardCharacter;

UCLASS(Abstract, Blueprintable)
class EPICWIZARDGAME_API ASpellBase : public AActor
{
	GENERATED_BODY()

public:

	ASpellBase();

	/** Base damage dealt by this spell */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float BaseDamage = 25.0f;

	/** Cooldown time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	float Cooldown = 1.0f;

	/** Display name of the spell */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spell")
	FString SpellName = "Spell";

	/** Execute the spell - override in child classes */
	UFUNCTION(BlueprintCallable, Category="Spell")
	virtual void Execute(AWizardCharacter* Caster);

	/** Can this spell be cast right now? */
	UFUNCTION(BlueprintPure, Category="Spell")
	bool CanCast() const;

	/** Get time remaining on cooldown */
	UFUNCTION(BlueprintPure, Category="Spell")
	float GetCooldownRemaining() const;

protected:

	virtual void BeginPlay() override;

	/** Reference to the wizard who owns this spell */
	UPROPERTY()
	AWizardCharacter* OwnerWizard;

	/** Time when this spell was last cast */
	float LastCastTime = -999.0f;

	/** Start cooldown timer */
	void StartCooldown();
};
