// Fill out your copyright notice in the Description page of Project Settings.

#include "SpellProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ZombieCharacter.h"
#include "Engine/DamageEvents.h"

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
		FDamageEvent DamageEvent;
		Zombie->TakeDamage(Damage, DamageEvent, GetInstigatorController(), this);
		UE_LOG(LogTemp, Log, TEXT("Projectile hit zombie for %f damage"), Damage);
	}

	// Destroy projectile on hit
	Destroy();
}

