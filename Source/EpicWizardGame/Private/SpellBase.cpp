// Fill out your copyright notice in the Description page of Project Settings.

#include "SpellBase.h"
#include "WizardCharacter.h"

ASpellBase::ASpellBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASpellBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASpellBase::Execute(AWizardCharacter* Caster)
{
	// Base implementation does nothing - override in child classes
	if (!Caster)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpellBase::Execute - No caster provided!"));
		return;
	}

	if (!CanCast())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpellBase::Execute - Spell on cooldown!"));
		return;
	}

	OwnerWizard = Caster;
	StartCooldown();
}

bool ASpellBase::CanCast() const
{
	float TimeSinceLastCast = GetWorld()->GetTimeSeconds() - LastCastTime;
	return TimeSinceLastCast >= Cooldown;
}

float ASpellBase::GetCooldownRemaining() const
{
	float TimeSinceLastCast = GetWorld()->GetTimeSeconds() - LastCastTime;
	float Remaining = Cooldown - TimeSinceLastCast;
	return FMath::Max(0.0f, Remaining);
}

void ASpellBase::StartCooldown()
{
	LastCastTime = GetWorld()->GetTimeSeconds();
}

