// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WizardPlayerController.generated.h"

/**
 * Player controller for the Wizard character
 */
UCLASS()
class EPICWIZARDGAME_API AWizardPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AWizardPlayerController();

protected:

	virtual void BeginPlay() override;
};
