#include "TurretAir.h"
#include "ZombieCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"

ATurretAir::ATurretAir()
{
	// Mirror airblast spell pacing and damage
	FireRate = 1.5f;
	ProjectileDamage = 10.0f;
	ProjectileVerticalOffset = 80.0f;
	AirProjectileClass = AActor::StaticClass();
}

void ATurretAir::ShootAtTarget(AZombieCharacter* Target)
{
	if (!Target)
	{
		return;
	}

	if (!AirProjectileClass)
	{
		return;
	}

	FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	// Spawn slightly forward and raised so the blast doesn't originate at the base
	FVector SpawnLocation = GetActorLocation()
		+ (Direction * 100.0f)
		+ FVector(0.0f, 0.0f, ProjectileVerticalOffset);

	FRotator SpawnRotation = Direction.Rotation();
	SpawnRotation.Pitch += ProjectileVisualPitchOffset; // visual-only tweak

	AActor* AirblastProjectile = GetWorld()->SpawnActor<AActor>(AirProjectileClass, SpawnLocation, SpawnRotation);
	if (!AirblastProjectile)
	{
		return;
	}

	AirblastProjectile->SetLifeSpan(ProjectileLifetime);

	FVector Velocity = Direction * ProjectileSpeed;
	TWeakObjectPtr<AActor> WeakProjectile = AirblastProjectile;
	TWeakObjectPtr<ATurretAir> WeakTurret = this;
	const float Damage = ProjectileDamage;
	const float Knockback = KnockbackForce;
	const float Radius = BlastRadius;

	FTimerDelegate TickDelegate;
	TickDelegate.BindLambda([WeakProjectile, WeakTurret, Velocity, Damage, Knockback, Radius]()
	{
		if (!WeakProjectile.IsValid() || !WeakTurret.IsValid())
		{
			return;
		}

		AActor* Projectile = WeakProjectile.Get();
		FVector NewLocation = Projectile->GetActorLocation() + (Velocity * 0.016f);
		Projectile->SetActorLocation(NewLocation);

		// Check for zombies to damage/knock back
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(Projectile->GetWorld(), AZombieCharacter::StaticClass(), FoundActors);

		for (AActor* Actor : FoundActors)
		{
			AZombieCharacter* Zombie = Cast<AZombieCharacter>(Actor);
			if (!Zombie || Zombie->IsDead())
			{
				continue;
			}

			if (FVector::Dist(Projectile->GetActorLocation(), Zombie->GetActorLocation()) <= Radius)
			{
				FDamageEvent DamageEvent;
				Zombie->TakeDamage(Damage, DamageEvent, WeakTurret->GetInstigatorController(), Projectile);

				FVector KnockbackDirection = (Zombie->GetActorLocation() - Projectile->GetActorLocation()).GetSafeNormal();
				KnockbackDirection.Z = 0.0f; // match Airblast: horizontal knockback only
				KnockbackDirection.Normalize();
				Zombie->LaunchCharacter(KnockbackDirection * Knockback, true, true);
			}
		}
	});

	FTimerHandle TickTimer;
	GetWorld()->GetTimerManager().SetTimer(TickTimer, TickDelegate, 0.016f, true);
}
