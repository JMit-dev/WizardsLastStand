// Fill out your copyright notice in the Description page of Project Settings.

#include "IceSpell.h"
#include "WizardCharacter.h"
#include "ZombieCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"

AIceSpell::AIceSpell()
{
	SpellName = "Ice Cone";
	BaseDamage = 15.0f;
	Cooldown = 2.0f;
}

void AIceSpell::Execute(AWizardCharacter* Caster)
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
		FVector ToZombie = Zombie->GetActorLocation() - CameraLocation;
		float Distance = ToZombie.Size();

		if (Distance > ConeRange)
		{
			continue;
		}

		// Check if zombie is in cone
		ToZombie.Normalize();
		float DotProduct = FVector::DotProduct(CameraForward, ToZombie);
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

	// Spawn blue cone visual that shoots forward
	FVector SpawnLocation = CameraLocation + (CameraForward * 100.0f);
	AActor* IceCone = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, CameraForward.Rotation());
	if (IceCone)
	{
		UStaticMeshComponent* ConeMesh = NewObject<UStaticMeshComponent>(IceCone);
		ConeMesh->RegisterComponent();
		ConeMesh->SetupAttachment(IceCone->GetRootComponent());

		// Cone shape pointing forward, blue and semi-transparent
		FVector ConeScale(ConeRange / 100.0f, 2.0f, 2.0f);
		ConeMesh->SetRelativeScale3D(ConeScale);
		ConeMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

		IceCone->SetLifeSpan(0.5f);
	}
}

