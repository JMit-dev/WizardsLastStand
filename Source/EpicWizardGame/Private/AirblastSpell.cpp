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
	BaseDamage = 5.0f;
	Cooldown = 1.5f;
}

void AAirblastSpell::Execute(AWizardCharacter* Caster)
{
	Super::Execute(Caster);

	if (!Caster)
	{
		return;
	}

	// Get camera direction
	UCameraComponent* Camera = Caster->GetFirstPersonCamera();
	if (!Camera)
	{
		return;
	}

	FVector CameraLocation = Camera->GetComponentLocation();
	FVector CameraForward = Camera->GetForwardVector();
	FVector SpawnLocation = CameraLocation + (CameraForward * 100.0f);

	// Spawn wide horizontal rectangle projectile
	AActor* AirblastProjectile = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, CameraForward.Rotation());
	if (!AirblastProjectile)
	{
		return;
	}

	// Add mesh component (wide horizontal rectangle)
	UStaticMeshComponent* RectMesh = NewObject<UStaticMeshComponent>(AirblastProjectile);
	RectMesh->RegisterComponent();
	RectMesh->SetupAttachment(AirblastProjectile->GetRootComponent());

	// Wide and short (horizontal rectangle)
	FVector RectScale(1.0f, 4.0f, 3.0f); // Wide horizontally, shorter vertically
	RectMesh->SetRelativeScale3D(RectScale);

	// Set projectile to move forward and destroy after lifetime
	AirblastProjectile->SetLifeSpan(ProjectileLifetime);

	// Store initial data for tick-based movement
	FVector Velocity = CameraForward * ProjectileSpeed;

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

				// Apply knockback
				FVector KnockbackDirection = (Zombie->GetActorLocation() - Projectile->GetActorLocation()).GetSafeNormal();
				KnockbackDirection.Z = 0.3f;

				UCharacterMovementComponent* MovementComp = Zombie->GetCharacterMovement();
				if (MovementComp)
				{
					MovementComp->AddImpulse(KnockbackDirection * KnockbackForce, true);
				}
			}
		}
	});

	GetWorld()->GetTimerManager().SetTimer(TickTimer, TickDelegate, 0.016f, true);

	UE_LOG(LogTemp, Log, TEXT("Airblast projectile launched"));
}

