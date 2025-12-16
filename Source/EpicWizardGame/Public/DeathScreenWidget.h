// Simple death screen widget built in C++ to mirror the title screen flow.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathScreenWidget.generated.h"

/**
 * Minimal death screen UI that displays a message and a "Try Again" button.
 * The button returns the player to the TitleScreen level.
 */
UCLASS()
class EPICWIZARDGAME_API UDeathScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDeathScreenWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FReply HandleTryAgainClicked();
};
