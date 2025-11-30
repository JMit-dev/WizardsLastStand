#include "TurretLightning.h"
#include "ZombieCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "SpellProjectile.h"
#include "Engine/DamageEvents.h"

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
		FVector TurretLocation = GetActorLocation();
		FVector Direction = (StrikeLocation - TurretLocation).GetSafeNormal();
		FVector SpawnLocation = TurretLocation + FVector(0.0f, 0.0f, 50.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		if (ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(
			ProjectileClass,
			SpawnLocation,
			Direction.Rotation(),
			SpawnParams))
		{
			Projectile->InitializeProjectile(Direction, 0.0f);
		}
	}
}
