// Fill out your copyright notice in the Description page of Project Settings.

#include "SpellProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ZombieCharacter.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"

ASpellProjectile::ASpellProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create collision sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(15.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	RootComponent = CollisionSphere;

	// Create mesh
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create projectile movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// Bind hit event
	CollisionSphere->OnComponentHit.AddDynamic(this, &ASpellProjectile::OnHit);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASpellProjectile::OnOverlap);
}

void ASpellProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Auto-destroy after lifetime
	SetLifeSpan(Lifetime);
}

void ASpellProjectile::InitializeProjectile(const FVector& Direction, float InDamage)
{
	Damage = InDamage;
	ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;

	if (bPierceTargets)
	{
		// Overlap everything so we don't stop on impact, handle damage in overlap
		CollisionSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionSphere->SetGenerateOverlapEvents(true);
	}
	else
	{
		// Default: block pawns so hit events fire and destroy on impact
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		CollisionSphere->SetGenerateOverlapEvents(false);
		CollisionSphere->SetNotifyRigidBodyCollision(true);
	}
}

void ASpellProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	// Deal damage to zombies
	if (AZombieCharacter* Zombie = Cast<AZombieCharacter>(OtherActor))
	{
		const float Speed = ProjectileMovement ? ProjectileMovement->Velocity.Size() : 0.0f;

		if (bPierceTargets)
		{
			const bool bAlreadyHit = PiercedActors.ContainsByPredicate([Zombie](const TWeakObjectPtr<AActor>& Ptr)
			{
				return Ptr.Get() == Zombie;
			});

			if (bAlreadyHit)
			{
				return;
			}
		}

		FDamageEvent DamageEvent;
		Zombie->TakeDamage(Damage, DamageEvent, GetInstigatorController(), this);
		UE_LOG(LogTemp, Log, TEXT("Projectile hit zombie for %f damage"), Damage);

		// Apply optional freeze/slow effect
		if (bApplyFreeze && FreezeDuration > 0.0f && FreezeSpeedMultiplier >= 0.0f)
		{
			if (UCharacterMovementComponent* MovementComp = Zombie->GetCharacterMovement())
			{
				const float OriginalSpeed = MovementComp->MaxWalkSpeed;
				MovementComp->MaxWalkSpeed = OriginalSpeed * FreezeSpeedMultiplier;

				FTimerHandle FreezeTimer;
				TWeakObjectPtr<UCharacterMovementComponent> MovementWeak = MovementComp;
				GetWorld()->GetTimerManager().SetTimer(FreezeTimer, [MovementWeak, OriginalSpeed]()
				{
					if (MovementWeak.IsValid())
					{
						MovementWeak->MaxWalkSpeed = OriginalSpeed;
					}
				}, FreezeDuration, false);
			}
		}

		// For piercing projectiles, ignore this zombie and keep flying
		if (bPierceTargets && ProjectileMovement)
		{
			// Prevent re-hitting the same target
			CollisionSphere->IgnoreActorWhenMoving(Zombie, true);
			PiercedActors.Add(Zombie);
			const FVector Direction = ProjectileMovement->Velocity.GetSafeNormal();
			ProjectileMovement->Velocity = Direction * Speed;
			ProjectileMovement->UpdateComponentVelocity();
			return;
		}
	}

	// Destroy projectile on hit
	if (!bPierceTargets)
	{
		Destroy();
	}
}

void ASpellProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bPierceTargets)
	{
		return; // overlap path is for piercing projectiles
	}

	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	if (AZombieCharacter* Zombie = Cast<AZombieCharacter>(OtherActor))
	{
		const bool bAlreadyHit = PiercedActors.ContainsByPredicate([Zombie](const TWeakObjectPtr<AActor>& Ptr)
		{
			return Ptr.Get() == Zombie;
		});

		if (bAlreadyHit)
		{
			return;
		}

		FDamageEvent DamageEvent;
		Zombie->TakeDamage(Damage, DamageEvent, GetInstigatorController(), this);
		UE_LOG(LogTemp, Log, TEXT("Piercing projectile overlapped zombie for %f damage"), Damage);

		if (bApplyFreeze && FreezeDuration > 0.0f && FreezeSpeedMultiplier >= 0.0f)
		{
			if (UCharacterMovementComponent* MovementComp = Zombie->GetCharacterMovement())
			{
				const float OriginalSpeed = MovementComp->MaxWalkSpeed;
				MovementComp->MaxWalkSpeed = OriginalSpeed * FreezeSpeedMultiplier;

				FTimerHandle FreezeTimer;
				TWeakObjectPtr<UCharacterMovementComponent> MovementWeak = MovementComp;
				GetWorld()->GetTimerManager().SetTimer(FreezeTimer, [MovementWeak, OriginalSpeed]()
				{
					if (MovementWeak.IsValid())
					{
						MovementWeak->MaxWalkSpeed = OriginalSpeed;
					}
				}, FreezeDuration, false);
			}
		}

		// Prevent re-hitting the same target
		CollisionSphere->IgnoreActorWhenMoving(Zombie, true);
		PiercedActors.Add(Zombie);
	}
}

