#pragma once

#include "CoreMinimal.h"
#include "Turret.h"
#include "TurretAir.generated.h"

/**
 * Air turret - fires a short-lived blast that damages and knocks back nearby zombies
 */
UCLASS()
class EPICWIZARDGAME_API ATurretAir : public ATurret
{
	GENERATED_BODY()

public:
	ATurretAir();

protected:

	/** Projectile class to spawn for the airblast */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(DisplayName="Projectile Class"))
	TSubclassOf<AActor> AirProjectileClass;

	/** AOE radius around the airblast projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float BlastRadius = 400.0f;

	/** Knockback force strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float KnockbackForce = 2000.0f;

	/** Projectile forward speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float ProjectileSpeed = 1500.0f;

	/** Lifetime of the airblast projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float ProjectileLifetime = 1.0f;

	virtual void ShootAtTarget(AZombieCharacter* Target) override;
};
