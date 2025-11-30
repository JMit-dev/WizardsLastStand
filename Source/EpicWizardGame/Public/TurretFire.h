#pragma once

#include "CoreMinimal.h"
#include "Turret.h"
#include "TurretFire.generated.h"

/**
 * Fire turret - uses the base turret projectile behavior (fireball-style)
 */
UCLASS()
class EPICWIZARDGAME_API ATurretFire : public ATurret
{
	GENERATED_BODY()

public:
	ATurretFire();
};
