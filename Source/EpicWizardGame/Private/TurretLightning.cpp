#include "TurretLightning.h"
#include "ZombieCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "SpellProjectile.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"

ATurretLightning::ATurretLightning()
{
	FireRate = 3.0f;
	ProjectileDamage = 50.0f;
}

void ATurretLightning::ShootAtTarget(AZombieCharacter* Target)
{
	if (!Target)
	{
		return;
	}

	FVector StrikeLocation = Target->GetActorLocation();

	// Primary strike
	FDamageEvent DamageEvent;
	Target->TakeDamage(ProjectileDamage, DamageEvent, GetInstigatorController(), this);

	// Chain to nearby zombies
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombieCharacter::StaticClass(), FoundActors);

	float AOEDamage = ProjectileDamage * AOEDamageMultiplier;
	for (AActor* Actor : FoundActors)
	{
		AZombieCharacter* Zombie = Cast<AZombieCharacter>(Actor);
		if (!Zombie || Zombie == Target || Zombie->IsDead())
		{
			continue;
		}

		if (FVector::Dist(Zombie->GetActorLocation(), StrikeLocation) <= AOERadius)
		{
			Zombie->TakeDamage(AOEDamage, DamageEvent, GetInstigatorController(), this);
		}
	}

	// Visual projectile
	if (ProjectileClass)
	{
		// Spawn a visual strike above the target and send it straight down
		FVector SpawnLocation = StrikeLocation + FVector(0.0f, 0.0f, LightningStrikeHeight);
		FVector Direction = FVector(0.0f, 0.0f, -1.0f);
		FRotator SpawnRotation = Direction.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		if (ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(
			ProjectileClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams))
		{
			// Push the projectile downward quickly; damage already applied above
			if (UProjectileMovementComponent* Movement = Projectile->FindComponentByClass<UProjectileMovementComponent>())
			{
				const float OriginalSpeed = Movement->InitialSpeed;
				Movement->InitialSpeed = LightningStrikeSpeed;
				Movement->MaxSpeed = LightningStrikeSpeed;
				Projectile->InitializeProjectile(Direction, 0.0f);
				// Restore configured speed in case this projectile instance is reused elsewhere
				Movement->InitialSpeed = OriginalSpeed;
				Movement->MaxSpeed = OriginalSpeed;
			}
			else
			{
				Projectile->InitializeProjectile(Direction, 0.0f);
			}
		}
	}
}
