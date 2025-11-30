#pragma once

#include "CoreMinimal.h"
#include "Turret.h"
#include "TurretLightning.generated.h"

/**
 * Lightning turret - strikes a target for heavy damage and chains in a small AOE
 */
UCLASS()
class EPICWIZARDGAME_API ATurretLightning : public ATurret
{
	GENERATED_BODY()

public:
	ATurretLightning();

protected:

	/** AOE radius around the strike point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AOERadius = 300.0f;

	/** Multiplier applied to AOE damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float AOEDamageMultiplier = 0.5f;

	virtual void ShootAtTarget(AZombieCharacter* Target) override;
};
