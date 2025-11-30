// Fill out your copyright notice in the Description page of Project Settings.

#include "LightningSpell.h"
#include "WizardCharacter.h"
#include "ZombieCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "SpellProjectile.h"
#include "Engine/DamageEvents.h"

ALightningSpell::ALightningSpell()
{
	SpellName = "Lightning Strike";
	BaseDamage = 50.0f;
	Cooldown = 3.0f;
}

void ALightningSpell::Execute(AWizardCharacter* Caster)
{
	Super::Execute(Caster);

	if (!Caster)
	{
		return;
	}

	// Get camera forward vector
	UCameraComponent* Camera = Caster->GetFirstPersonCamera();
	if (!Camera)
	{
		return;
	}

	FVector CameraLocation = Camera->GetComponentLocation();
	FVector CameraForward = Camera->GetForwardVector();

	// Raycast forward to find a zombie
	FHitResult HitResult;
	FVector TraceEnd = CameraLocation + (CameraForward * RaycastRange);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Caster);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CameraLocation,
		TraceEnd,
		ECC_Pawn,
		QueryParams
	);

	if (!bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Lightning: No target found"));
		return;
	}

	// Check if we hit a zombie
	AZombieCharacter* TargetZombie = Cast<AZombieCharacter>(HitResult.GetActor());
	if (!TargetZombie)
	{
		UE_LOG(LogTemp, Warning, TEXT("Lightning: Hit non-zombie target"));
		return;
	}

	FVector StrikeLocation = HitResult.Location;

	// Deal damage to primary target
	FDamageEvent DamageEvent;
	TargetZombie->TakeDamage(BaseDamage, DamageEvent, Caster->GetController(), this);

	UE_LOG(LogTemp, Log, TEXT("Lightning struck %s for %f damage!"), *TargetZombie->GetName(), BaseDamage);

	// Find all zombies in AOE radius
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombieCharacter::StaticClass(), FoundActors);

	int32 AOEHits = 0;
	float AOEDamage = BaseDamage * AOEDamageMultiplier;

	for (AActor* Actor : FoundActors)
	{
		AZombieCharacter* Zombie = Cast<AZombieCharacter>(Actor);
		if (!Zombie || Zombie == TargetZombie)
		{
			continue;
		}

		float Distance = FVector::Dist(Zombie->GetActorLocation(), StrikeLocation);
		if (Distance <= AOERadius)
		{
			Zombie->TakeDamage(AOEDamage, DamageEvent, Caster->GetController(), this);
			AOEHits++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Lightning AOE hit %d additional zombies"), AOEHits);

	// Spawn projectile visual (lightning bolt) similar to other spells
	if (ProjectileClass)
	{
		FVector SpawnLocation = CameraLocation + (CameraForward * SpawnDistance);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Caster;
		SpawnParams.Instigator = Caster;

		if (ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(
			ProjectileClass,
			SpawnLocation,
			CameraForward.Rotation(),
			SpawnParams))
		{
			// Damage already applied via raycast/AOE
			Projectile->InitializeProjectile(CameraForward, 0.0f);
		}
	}

	// Debug visualization
	DrawDebugSphere(GetWorld(), StrikeLocation, AOERadius, 12, FColor::Yellow, false, 2.0f);
}

