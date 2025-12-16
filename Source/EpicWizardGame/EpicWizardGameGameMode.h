// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EpicWizardGameGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AEpicWizardGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEpicWizardGameGameMode();

protected:
	virtual void StartPlay() override;
};


