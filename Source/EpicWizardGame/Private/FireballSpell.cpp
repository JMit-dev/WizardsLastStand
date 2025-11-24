// Fill out your copyright notice in the Description page of Project Settings.

#include "FireballSpell.h"
#include "SpellProjectile.h"
#include "WizardCharacter.h"
#include "Camera/CameraComponent.h"

AFireballSpell::AFireballSpell()
{
	SpellName = "Fireball";
	BaseDamage = 30.0f;
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

	// Get camera forward vector
	UCameraComponent* Camera = Caster->GetFirstPersonCamera();
	if (!Camera)
	{
		UE_LOG(LogTemp, Warning, TEXT("FireballSpell::Execute - No camera found!"));
		return;
	}

	FVector CameraLocation = Camera->GetComponentLocation();
	FVector CameraForward = Camera->GetForwardVector();

	// Spawn location slightly in front of camera
	FVector SpawnLocation = CameraLocation + (CameraForward * SpawnDistance);

	// Spawn projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Caster;
	SpawnParams.Instigator = Caster;

	ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(
		ProjectileClass,
		SpawnLocation,
		CameraForward.Rotation(),
		SpawnParams
	);

	if (Projectile)
	{
		Projectile->InitializeProjectile(CameraForward, BaseDamage);
		UE_LOG(LogTemp, Log, TEXT("Fireball cast!"));
	}
}

