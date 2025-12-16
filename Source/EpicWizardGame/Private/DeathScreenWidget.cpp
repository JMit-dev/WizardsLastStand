// Simple death screen widget built in C++ to mirror the title screen flow.

#include "DeathScreenWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/CoreStyle.h"
#include "Kismet/GameplayStatics.h"

UDeathScreenWidget::UDeathScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
TSharedRef<SWidget> UDeathScreenWidget::RebuildWidget()
{
	// Build a simple centered panel with a title, subtitle, and button.
	const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 48);
	const FSlateFontInfo BodyFont = FCoreStyle::GetDefaultFontStyle("Regular", 20);
	const FSlateFontInfo ButtonFont = FCoreStyle::GetDefaultFontStyle("Bold", 24);

	return SNew(SOverlay)
	+ SOverlay::Slot()
	[
		SNew(SBorder)
		.Padding(FMargin(0.0f))
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.7f)))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SBox)
				.MinDesiredWidth(520.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("DeathScreen", "Title", "YOU DIED!"))
						.Font(TitleFont)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.05f, 0.05f, 1.0f)))
					]
					// Spacer where subtitle was, preserving layout
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(FMargin(0.f, 12.f, 0.f, 0.f))
					[
						SNew(SBox)
						.MinDesiredHeight(8.f)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(FMargin(0.f, 26.f, 0.f, 0.f))
					[
						SNew(SButton)
						.OnClicked(FOnClicked::CreateUObject(this, &UDeathScreenWidget::HandleTryAgainClicked))
						.ContentPadding(FMargin(26.f, 12.f))
						.ButtonColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.1f, 0.1f, 1.f)))
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("DeathScreen", "TryAgain", "Dare to cast spells once more?"))
							.Font(ButtonFont)
							.Justification(ETextJustify::Center)
							.ColorAndOpacity(FSlateColor(FLinearColor::White))
						]
					]
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply UDeathScreenWidget::HandleTryAgainClicked()
{
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, FName("TitleScreen"));
	}
	return FReply::Handled();
}
