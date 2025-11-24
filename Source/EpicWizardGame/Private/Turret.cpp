// Fill out your copyright notice in the Description page of Project Settings.


#include "Turret.h"
#include "ZombieCharacter.h"
#include "SpellProjectile.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATurret::ATurret()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATurret::BeginPlay()
{
	Super::BeginPlay();
	FireTimer = FireRate;
}

// Called every frame
void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Find nearest zombie
	AZombieCharacter* NearestZombie = FindNearestZombie();

	// If we have a valid target
	if (NearestZombie && !NearestZombie->IsDead())
	{
		// Update fire timer
		FireTimer -= DeltaTime;

		// Shoot if timer is ready
		if (FireTimer <= 0.0f)
		{
			ShootAtTarget(NearestZombie);
			FireTimer = FireRate;
		}
	}
	else
	{
		// Reset timer when no target
		FireTimer = FireRate;
	}
}

// Called to bind functionality to input
void ATurret::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

AZombieCharacter* ATurret::FindNearestZombie()
{
	TArray<AActor*> FoundZombies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombieCharacter::StaticClass(), FoundZombies);

	AZombieCharacter* NearestZombie = nullptr;
	float NearestDistance = DetectionRange;

	for (AActor* Actor : FoundZombies)
	{
		AZombieCharacter* Zombie = Cast<AZombieCharacter>(Actor);
		if (Zombie && !Zombie->IsDead())
		{
			float Distance = FVector::Dist(GetActorLocation(), Zombie->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestZombie = Zombie;
			}
		}
	}

	return NearestZombie;
}

void ATurret::ShootAtTarget(AZombieCharacter* Target)
{
	if (!ProjectileClass || !Target)
	{
		return;
	}

	// Calculate direction to target
	FVector TurretLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector Direction = (TargetLocation - TurretLocation).GetSafeNormal();

	// Spawn projectile at turret location with offset upward
	FVector SpawnLocation = TurretLocation + FVector(0, 0, 50.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(ProjectileClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (Projectile)
	{
		// Initialize the projectile with direction and damage
		Projectile->InitializeProjectile(Direction, ProjectileDamage);
	}
}

