// Fill out your copyright notice in the Description page of Project Settings.

#include "IceSpell.h"
#include "WizardCharacter.h"
#include "ZombieCharacter.h"
#include "SpellProjectile.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

AIceSpell::AIceSpell()
{
	SpellName = "Ice Cone";
	// Lower damage than fireball, focus on freeze/crowd control
	// ~10-12 hits to kill round 1, main purpose is freeze
	BaseDamage = 15.0f;
	Cooldown = 1.5f;
}

void AIceSpell::Execute(AWizardCharacter* Caster)
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

	// Find all zombies in range
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombieCharacter::StaticClass(), FoundActors);

	int32 ZombiesFrozen = 0;

	for (AActor* Actor : FoundActors)
	{
		AZombieCharacter* Zombie = Cast<AZombieCharacter>(Actor);
		if (!Zombie)
		{
			continue;
		}

		// Check if zombie is in range
		FVector ToZombie = Zombie->GetActorLocation() - AimOrigin;
		float Distance = ToZombie.Size();

		if (Distance > ConeRange)
		{
			continue;
		}

		// Check if zombie is in cone
		ToZombie.Normalize();
		float DotProduct = FVector::DotProduct(AimDirection, ToZombie);
		float AngleRadians = FMath::Acos(DotProduct);
		float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

		if (AngleDegrees <= ConeAngle)
		{
			// Apply damage
			FDamageEvent DamageEvent;
			Zombie->TakeDamage(BaseDamage, DamageEvent, Caster->GetController(), this);

			// Apply freeze effect by slowing movement
			UCharacterMovementComponent* MovementComp = Zombie->GetCharacterMovement();
			if (MovementComp)
			{
				float OriginalSpeed = MovementComp->MaxWalkSpeed;
				MovementComp->MaxWalkSpeed *= FreezeSpeedMultiplier;

				// Reset speed after freeze duration
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

			ZombiesFrozen++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Ice spell cast! Froze %d zombies"), ZombiesFrozen);

	// Spawn projectile visual (ice shard) similar to fireball flow
	if (ProjectileClass)
	{
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
			// Damage is already applied by the cone; projectile here is for visuals/FX
			Projectile->InitializeProjectile(AimDirection, 0.0f);
		}
	}
}
