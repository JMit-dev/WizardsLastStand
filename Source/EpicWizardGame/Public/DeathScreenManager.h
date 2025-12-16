// Actor that spawns the death screen UI when placed in the DeathScreen level.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeathScreenManager.generated.h"

class UDeathScreenWidget;
class AActor;

/**
 * Drop this actor into the DeathScreen level to guarantee the death UI appears,
 * even if the level uses a Blueprint GameMode that doesn't inherit our C++ one.
 */
UCLASS()
class EPICWIZARDGAME_API ADeathScreenManager : public AActor
{
	GENERATED_BODY()

public:
	ADeathScreenManager();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UDeathScreenWidget* SpawnedWidget;

	/** Optional camera/view target to use for the death screen */
	UPROPERTY(EditInstanceOnly, Category="Death Screen|Camera")
	AActor* ViewTargetActor;
};
