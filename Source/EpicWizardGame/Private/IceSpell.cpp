// Fill out your copyright notice in the Description page of Project Settings.

#include "IceSpell.h"
#include "WizardCharacter.h"
#include "SpellProjectile.h"

AIceSpell::AIceSpell()
{
	SpellName = "Ice Shard";
	// Lower damage than fireball, focuses on freeze/slow
	BaseDamage = 15.0f;
	Cooldown = 1.0f;
}

void AIceSpell::Execute(AWizardCharacter* Caster)
{
	Super::Execute(Caster);

	if (!Caster || !ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("IceSpell::Execute - Missing caster or projectile class!"));
		return;
	}

	FVector AimOrigin;
	FVector AimDirection;
	if (!Caster->GetAimData(AimOrigin, AimDirection))
	{
		UE_LOG(LogTemp, Warning, TEXT("IceSpell::Execute - No aim data available!"));
		return;
	}

	FVector SpawnLocation = AimOrigin + (AimDirection * SpawnDistance);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Caster;
	SpawnParams.Instigator = Caster;

	if (ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(
		ProjectileClass,
		SpawnLocation,
		AimDirection.Rotation(),
		SpawnParams))
	{
		// Configure freeze effect on the projectile so hits slow enemies
		Projectile->bApplyFreeze = true;
		Projectile->FreezeDuration = FreezeDuration;
		Projectile->FreezeSpeedMultiplier = FreezeSpeedMultiplier;
		Projectile->bPierceTargets = false; // fireball-style: stop on first enemy

		Projectile->InitializeProjectile(AimDirection, BaseDamage);
		UE_LOG(LogTemp, Log, TEXT("Ice shard fired"));
	}
}
