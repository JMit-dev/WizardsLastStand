// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildModeTimerWidget.h"
#include "WaveManager.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "EngineUtils.h"
#include "Blueprint/WidgetTree.h"

UBuildModeTimerWidget::UBuildModeTimerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// FORCE this widget to always tick
	bIsFocusable = false;
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UBuildModeTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Find the wave manager
	FindWaveManager();

	// Create TimerText programmatically if it doesn't exist
	if (!TimerText)
	{
		TimerText = NewObject<UTextBlock>(this);
		if (TimerText)
		{
			// Add to viewport
			UPanelWidget* RootWidget = Cast<UPanelWidget>(GetRootWidget());
			if (!RootWidget)
			{
				// Create a canvas panel as root if none exists
				UCanvasPanel* Canvas = NewObject<UCanvasPanel>(this);
				Canvas->SetFlags(RF_Transactional);
				WidgetTree->RootWidget = Canvas;
				RootWidget = Canvas;
			}

			if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(RootWidget))
			{
				UCanvasPanelSlot* CanvasSlot = Canvas->AddChildToCanvas(TimerText);
				if (CanvasSlot)
				{
					// Center the text
					CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
					CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
					CanvasSlot->SetPosition(FVector2D(0, 0));
					CanvasSlot->SetAutoSize(true);
				}
			}
		}
	}

	// Set initial font size and color
	if (TimerText)
	{
		// Make it VERY visible - bright yellow/orange
		TimerText->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 0.0f, 1.0f)); // Bright yellow

		FSlateFontInfo FontInfo = TimerText->GetFont();
		FontInfo.Size = FontSize;
		TimerText->SetFont(FontInfo);

		// Force update text immediately
		TimerText->SetText(FText::FromString(TEXT("BUILD MODE STARTING...")));
	}

	// Set widget size
	SetDesiredSizeInViewport(FVector2D(1920, 1080));
}

void UBuildModeTimerWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UBuildModeTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update the timer every frame
	UpdateTimer();
}

void UBuildModeTimerWidget::FindWaveManager()
{
	for (TActorIterator<AWaveManager> It(GetWorld()); It; ++It)
	{
		WaveManager = *It;
		break;
	}
}

void UBuildModeTimerWidget::UpdateTimer()
{
	if (!WaveManager)
	{
		// Try to find it if we don't have it yet
		FindWaveManager();

		if (!WaveManager)
		{
			return;
		}
	}

	if (!TimerText)
	{
		return;
	}

	// ALWAYS keep widget visible, just change the text
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	bool bInBuildMode = WaveManager->IsInBuildMode();

	if (bInBuildMode)
	{
		// Get remaining time and format it
		float RemainingTime = WaveManager->GetBuildModeTimeRemaining();
		int32 SecondsRemaining = FMath::CeilToInt(RemainingTime);

		// Create the text
		FText TimerDisplayText = FText::FromString(FString::Printf(TEXT("BUILD MODE: %d"), SecondsRemaining));
		TimerText->SetText(TimerDisplayText);

		// Force color - bright yellow
		TimerText->SetColorAndOpacity(FLinearColor(1.0f, 0.8f, 0.0f, 1.0f));
	}
	else
	{
		// Clear the text when not in build mode (but keep widget visible to keep ticking!)
		TimerText->SetText(FText::GetEmpty());
	}
}

