#include "TurretIce.h"
#include "ZombieCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SpellProjectile.h"
#include "Engine/DamageEvents.h"
#include "TimerManager.h"

ATurretIce::ATurretIce()
{
	FireRate = 2.0f;
	ProjectileDamage = 15.0f;
}

void ATurretIce::ShootAtTarget(AZombieCharacter* Target)
{
	if (!Target)
	{
		return;
	}

	FVector TurretLocation = GetActorLocation();
	// Keep the ice cone level so it doesn't pitch upward when targets are close
	FVector Forward = Target->GetActorLocation() - TurretLocation;
	Forward.Z = 0.0f;
	Forward = Forward.GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		Forward = GetActorForwardVector();
		Forward.Z = 0.0f;
		Forward = Forward.GetSafeNormal();
	}

	// Track if the primary target is actually inside the cone/range for visuals
	const float TargetDistance2D = FVector::Dist2D(Target->GetActorLocation(), TurretLocation);
	const bool bTargetInConeRange = TargetDistance2D <= ConeRange;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombieCharacter::StaticClass(), FoundActors);

	FDamageEvent DamageEvent;

	for (AActor* Actor : FoundActors)
	{
		AZombieCharacter* Zombie = Cast<AZombieCharacter>(Actor);
		if (!Zombie || Zombie->IsDead())
		{
			continue;
		}

		FVector ToZombie = Zombie->GetActorLocation() - TurretLocation;
		float Distance = ToZombie.Size();
		if (Distance > ConeRange)
		{
			continue;
		}

		FVector ToZombieFlat = ToZombie;
		ToZombieFlat.Z = 0.0f;
		if (!ToZombieFlat.Normalize())
		{
			continue;
		}

		float AngleRadians = FMath::Acos(FMath::Clamp(FVector::DotProduct(Forward, ToZombieFlat), -1.0f, 1.0f));
		float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

		if (AngleDegrees <= ConeAngle)
		{
			Zombie->TakeDamage(ProjectileDamage, DamageEvent, GetInstigatorController(), this);

			if (UCharacterMovementComponent* MovementComp = Zombie->GetCharacterMovement())
			{
				float OriginalSpeed = MovementComp->MaxWalkSpeed;
				MovementComp->MaxWalkSpeed = OriginalSpeed * FreezeSpeedMultiplier;

				FTimerHandle FreezeTimer;
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindLambda([MovementComp, OriginalSpeed]()
				{
					if (MovementComp)
					{
						MovementComp->MaxWalkSpeed = OriginalSpeed;
					}
				});

				GetWorld()->GetTimerManager().SetTimer(FreezeTimer, TimerDelegate, FreezeDuration, false);
			}
		}
	}

	// Quick ice cone visual
	if (ProjectileClass && bTargetInConeRange)
	{
		FVector SpawnLocation = TurretLocation
			+ (Forward * 100.0f)
			+ FVector(0.0f, 0.0f, ProjectileVerticalOffset);
		FRotator SpawnRotation = Forward.Rotation();
		SpawnRotation.Pitch += ProjectileVisualPitchOffset; // visual-only tweak

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		if (ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(
			ProjectileClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams))
		{
			// Visual-only shot; damage is handled by the cone application above
			Projectile->InitializeProjectile(Forward, 0.0f);
		}
	}
}
