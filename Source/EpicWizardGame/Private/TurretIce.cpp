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
	FVector Forward = (Target->GetActorLocation() - TurretLocation).GetSafeNormal();

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

		ToZombie.Normalize();
		float AngleRadians = FMath::Acos(FVector::DotProduct(Forward, ToZombie));
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
	if (ProjectileClass)
	{
		FVector SpawnLocation = TurretLocation + (Forward * 100.0f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		if (ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(
			ProjectileClass,
			SpawnLocation,
			Forward.Rotation(),
			SpawnParams))
		{
			// Visual-only shot; damage is handled by the cone application above
			Projectile->InitializeProjectile(Forward, 0.0f);
		}
	}
}
