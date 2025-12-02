// Fill out your copyright notice in the Description page of Project Settings.

#include "FireballSpell.h"
#include "SpellProjectile.h"
#include "WizardCharacter.h"
#include "Camera/CameraComponent.h"

AFireballSpell::AFireballSpell()
{
	SpellName = "Fireball";
	// Balanced for: 1-hit kill round 1 (150 HP), 4-5 hits round 10 (1050 HP)
	// 220 damage = 1 hit @ 150 HP, ~5 hits @ 1050 HP
	BaseDamage = 220.0f;
	Cooldown = 0.5f;
}

void AFireballSpell::Execute(AWizardCharacter* Caster)
{
	// Call parent to handle cooldown
	Super::Execute(Caster);

	if (!Caster || !ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("FireballSpell::Execute - Missing caster or projectile class!"));
		return;
	}

	FVector AimOrigin;
	FVector AimDirection;
	if (!Caster->GetAimData(AimOrigin, AimDirection))
	{
		UE_LOG(LogTemp, Warning, TEXT("FireballSpell::Execute - No aim data available!"));
		return;
	}

	// Spawn location slightly in front of camera
	FVector SpawnLocation = AimOrigin + (AimDirection * SpawnDistance);

	// Spawn projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Caster;
	SpawnParams.Instigator = Caster;

	ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(
		ProjectileClass,
		SpawnLocation,
		AimDirection.Rotation(),
		SpawnParams
	);

	if (Projectile)
	{
		Projectile->InitializeProjectile(AimDirection, BaseDamage);
		UE_LOG(LogTemp, Log, TEXT("Fireball cast!"));
	}
}

