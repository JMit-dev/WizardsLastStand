#pragma once

#include "CoreMinimal.h"
#include "Turret.h"
#include "TurretIce.generated.h"

class ASpellProjectile;

/**
 * Ice turret - fires a freezing projectile like the wizard's Ice spell
 */
UCLASS()
class EPICWIZARDGAME_API ATurretIce : public ATurret
{
	GENERATED_BODY()

public:
	ATurretIce();

protected:
	// Fire straight (flat) but with the ice projectile
	virtual void ShootAtTarget(AZombieCharacter* Target) override;

	/** Freeze duration in seconds (matches Ice spell) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float FreezeDuration = 3.0f;

	/** Movement speed multiplier when frozen (matches Ice spell) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float FreezeSpeedMultiplier = 0.2f;
};
