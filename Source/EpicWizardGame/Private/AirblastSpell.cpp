// Fill out your copyright notice in the Description page of Project Settings.

#include "AirblastSpell.h"
#include "WizardCharacter.h"
#include "ZombieCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/DamageEvents.h"

AAirblastSpell::AAirblastSpell()
{
	SpellName = "Airblast";
	// Medium damage with strong horizontal knockback
	// ~5-6 hits to kill round 1, main purpose is crowd control
	BaseDamage = 10.0f;
	Cooldown = 1.5f;
	ProjectileClass = AActor::StaticClass();
}

void AAirblastSpell::Execute(AWizardCharacter* Caster)
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

	FVector SpawnLocation = AimOrigin + (AimDirection * 100.0f);

	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AirblastSpell::Execute - No projectile class set"));
		return;
	}

	// Spawn wide horizontal rectangle projectile (allows custom classes)
	AActor* AirblastProjectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, AimDirection.Rotation());
	if (!AirblastProjectile)
	{
		return;
	}

	// Add a simple default mesh when using the bare AActor class
	if (ProjectileClass == AActor::StaticClass())
	{
		UStaticMeshComponent* RectMesh = NewObject<UStaticMeshComponent>(AirblastProjectile);
		RectMesh->RegisterComponent();
		RectMesh->SetupAttachment(AirblastProjectile->GetRootComponent());

		// Wide and short (horizontal rectangle)
		FVector RectScale(1.0f, 4.0f, 3.0f); // Wide horizontally, shorter vertically
		RectMesh->SetRelativeScale3D(RectScale);
	}

	// Set projectile to move forward and destroy after lifetime
	AirblastProjectile->SetLifeSpan(ProjectileLifetime);

	// Store initial data for tick-based movement
	FVector Velocity = AimDirection * ProjectileSpeed;

	// Lambda to handle movement and collision each tick
	FTimerHandle TickTimer;
	FTimerDelegate TickDelegate;

	TWeakObjectPtr<AActor> WeakProjectile = AirblastProjectile;
	TWeakObjectPtr<AWizardCharacter> WeakCaster = Caster;

	TickDelegate.BindLambda([WeakProjectile, WeakCaster, Velocity, this]()
	{
		if (!WeakProjectile.IsValid() || !WeakCaster.IsValid())
		{
			return;
		}

		AActor* Projectile = WeakProjectile.Get();
		AWizardCharacter* WizCaster = WeakCaster.Get();

		// Move projectile forward
		FVector NewLocation = Projectile->GetActorLocation() + (Velocity * 0.016f);
		Projectile->SetActorLocation(NewLocation);

		// Check for zombies in range and knock them back
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(Projectile->GetWorld(), AZombieCharacter::StaticClass(), FoundActors);

		for (AActor* Actor : FoundActors)
		{
			AZombieCharacter* Zombie = Cast<AZombieCharacter>(Actor);
			if (!Zombie)
			{
				continue;
			}

			float Distance = FVector::Dist(Projectile->GetActorLocation(), Zombie->GetActorLocation());
			if (Distance <= BlastRadius)
			{
				// Apply damage
				FDamageEvent DamageEvent;
				Zombie->TakeDamage(BaseDamage, DamageEvent, WizCaster->GetController(), Projectile);

				// Apply horizontal knockback only (no vertical lift)
				FVector KnockbackDirection = (Zombie->GetActorLocation() - Projectile->GetActorLocation()).GetSafeNormal();
				KnockbackDirection.Z = 0.0f; // Remove vertical component for pure horizontal push
				KnockbackDirection.Normalize();

				// Use LaunchCharacter so AI velocity is overridden for a noticeable shove
				Zombie->LaunchCharacter(KnockbackDirection * KnockbackForce, true, true);
			}
		}
	});

	GetWorld()->GetTimerManager().SetTimer(TickTimer, TickDelegate, 0.016f, true);

	UE_LOG(LogTemp, Log, TEXT("Airblast projectile launched"));
}

