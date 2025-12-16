// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WizardPlayerController.generated.h"

class UHotbarWidget;
class UDeathScreenWidget;

/**
 * Player controller for the Wizard character
 */
UCLASS()
class EPICWIZARDGAME_API AWizardPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AWizardPlayerController();

	/** Set reference to the hotbar widget */
	UFUNCTION(BlueprintCallable, Category="UI")
	void SetHotbarWidget(UHotbarWidget* Widget);

	/** Get reference to the hotbar widget */
	UFUNCTION(BlueprintPure, Category="UI")
	UHotbarWidget* GetHotbarWidget() const { return HotbarWidget; }

protected:

	virtual void BeginPlay() override;

	/** Reference to the hotbar widget */
	UPROPERTY()
	UHotbarWidget* HotbarWidget;

	/** Reference to the death screen widget (only used on the DeathScreen level) */
	UPROPERTY()
	UDeathScreenWidget* DeathScreenWidget;
};
