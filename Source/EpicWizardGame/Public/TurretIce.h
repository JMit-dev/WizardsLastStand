#pragma once

#include "CoreMinimal.h"
#include "Turret.h"
#include "TurretIce.generated.h"

/**
 * Ice turret - applies a freezing cone in front of the turret
 */
UCLASS()
class EPICWIZARDGAME_API ATurretIce : public ATurret
{
	GENERATED_BODY()

public:
	ATurretIce();

protected:

	/** Cone range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float ConeRange = 500.0f;

	/** Cone angle in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float ConeAngle = 45.0f;

	/** Freeze duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float FreezeDuration = 3.0f;

	/** Movement speed multiplier when frozen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float FreezeSpeedMultiplier = 0.2f;

	virtual void ShootAtTarget(AZombieCharacter* Target) override;
};
