// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildModeTimerWidget.generated.h"

class AWaveManager;
class UTextBlock;

/**
 * Giant countdown timer displayed in the center of screen during build mode
 */
UCLASS()
class EPICWIZARDGAME_API UBuildModeTimerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UBuildModeTimerWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	/** Text block for displaying the countdown (bind this in BP) */
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* TimerText;

	/** Font size for the timer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timer")
	int32 FontSize = 80;

	/** Text color during build mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timer")
	FLinearColor TimerColor = FLinearColor::Yellow;

protected:
	/** Cached reference to wave manager */
	AWaveManager* WaveManager;

	/** Find the wave manager in the level */
	void FindWaveManager();

	/** Update the timer display */
	void UpdateTimer();
};
