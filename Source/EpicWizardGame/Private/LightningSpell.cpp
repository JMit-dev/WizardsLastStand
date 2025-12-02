// Fill out your copyright notice in the Description page of Project Settings.

#include "LightningSpell.h"
#include "WizardCharacter.h"
#include "ZombieCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "SpellProjectile.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"

ALightningSpell::ALightningSpell()
{
	SpellName = "Lightning Strike";
	// High damage AOE spell with slow recovery
	// ~2-3 hits to kill round 1, plus AOE damage to nearby zombies
	BaseDamage = 75.0f;
	Cooldown = 4.0f; // Slow recovery time
}

void ALightningSpell::Execute(AWizardCharacter* Caster)
{
	Super::Execute(Caster);

	if (!Caster)
	{
		return;
	}

	FVector AimOrigin;
	FVector AimDirection;
	if (!Caster->GetAimData(AimOrigin, AimDirection))
	{
		return;
	}

	// Raycast forward to find a zombie
	FHitResult HitResult;
	FVector TraceEnd = AimOrigin + (AimDirection * RaycastRange);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Caster);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		AimOrigin,
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

	// Spawn projectile visual (lightning bolt) that drops from above the strike point
	if (ProjectileClass)
	{
		FVector SpawnLocation = StrikeLocation + FVector(0.0f, 0.0f, LightningStrikeHeight);
		FVector Direction = FVector(0.0f, 0.0f, -1.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Caster;
		SpawnParams.Instigator = Caster;

		if (ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(
			ProjectileClass,
			SpawnLocation,
			Direction.Rotation(),
			SpawnParams))
		{
			// Damage already applied via raycast/AOE; this is visual-only
			if (UProjectileMovementComponent* Movement = Projectile->FindComponentByClass<UProjectileMovementComponent>())
			{
				const float OriginalSpeed = Movement->InitialSpeed;
				Movement->InitialSpeed = LightningStrikeSpeed;
				Movement->MaxSpeed = LightningStrikeSpeed;
				Projectile->InitializeProjectile(Direction, 0.0f);
				// Restore configured speed for future use of this projectile class
				Movement->InitialSpeed = OriginalSpeed;
				Movement->MaxSpeed = OriginalSpeed;
			}
			else
			{
				Projectile->InitializeProjectile(Direction, 0.0f);
			}
		}
	}

}

