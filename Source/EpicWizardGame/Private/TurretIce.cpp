#include "TurretIce.h"
#include "ZombieCharacter.h"
#include "SpellProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

ATurretIce::ATurretIce()
{
	// Match the fire turret behavior but swap in the ice projectile
	FireRate = 1.0f;      // match Ice spell cooldown
	ProjectileDamage = 15.0f; // match Ice spell base damage
	ProjectileAimPitchOffset = 0.0f;
	ProjectileVisualPitchOffset = 0.0f;

	static ConstructorHelpers::FClassFinder<ASpellProjectile> IceProjBP(TEXT("/Game/WizardsLastStand/Blueprints/BP_IceProjectile"));
	if (IceProjBP.Succeeded())
	{
		ProjectileClass = IceProjBP.Class;
	}
}

void ATurretIce::ShootAtTarget(AZombieCharacter* Target)
{
	if (!ProjectileClass || !Target)
	{
		return;
	}

	// Flatten aim so the shard fires straight horizontally
	FVector TurretLocation = GetActorLocation();
	FVector Direction = Target->GetActorLocation() - TurretLocation;
	Direction.Z = 0.0f;
	Direction = Direction.GetSafeNormal();

	if (!Direction.IsNearlyZero())
	{
		const FVector SpawnLocation = TurretLocation + FVector(0, 0, ProjectileVerticalOffset);
		const FRotator SpawnRotation(0.0f, Direction.Rotation().Yaw, 0.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		if (ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams))
		{
			// Speed up the ice projectile for better accuracy
			if (UProjectileMovementComponent* MoveComp = Projectile->FindComponentByClass<UProjectileMovementComponent>())
			{
				MoveComp->InitialSpeed = 2500.0f;
				MoveComp->MaxSpeed = 2500.0f;
			}

			// Apply slow like the Ice spell
			Projectile->bApplyFreeze = true;
			Projectile->FreezeDuration = FreezeDuration;
			Projectile->FreezeSpeedMultiplier = FreezeSpeedMultiplier;
			Projectile->bPierceTargets = false;

			Projectile->InitializeProjectile(Direction, ProjectileDamage);

			// Re-apply velocity in case movement settings changed above
			if (UProjectileMovementComponent* MoveComp = Projectile->FindComponentByClass<UProjectileMovementComponent>())
			{
				MoveComp->Velocity = Direction * MoveComp->InitialSpeed;
				MoveComp->UpdateComponentVelocity();
			}
		}
	}
}
