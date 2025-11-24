// Fill out your copyright notice in the Description page of Project Settings.

#include "HotbarWidget.h"
#include "Components/Border.h"
#include "SpellBase.h"
#include "WizardCharacter.h"
#include "WizardPlayerController.h"
#include "EngineUtils.h"

void UHotbarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start with first slot selected
	CurrentSlotIndex = 0;
	UpdateVisuals();
}

FReply UHotbarWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Input is now handled by Player Controller, not the widget
	// This prevents focus issues
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UHotbarWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Input is now handled by Player Controller, not the widget
	// This prevents focus issues
	return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

FReply UHotbarWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Input is now handled by Player Controller, not the widget
	// This prevents focus issues
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UHotbarWidget::SelectNextSlot()
{
	int32 NewIndex = CurrentSlotIndex + 1;
	SelectSlot(NewIndex);
}

void UHotbarWidget::SelectPreviousSlot()
{
	int32 NewIndex = CurrentSlotIndex - 1;
	SelectSlot(NewIndex);
}

void UHotbarWidget::SelectSlot(int32 SlotIndex)
{
	int32 NewIndex = ClampSlotIndex(SlotIndex);

	if (NewIndex != CurrentSlotIndex)
	{
		CurrentSlotIndex = NewIndex;
		UpdateVisuals();
		OnSlotChanged.Broadcast(CurrentSlotIndex);

		UE_LOG(LogTemp, Log, TEXT("HotbarWidget: Selected slot %d"), CurrentSlotIndex + 1);
	}
}

void UHotbarWidget::SelectSlotByNumber(int32 SlotNumber)
{
	// Convert slot number (1-5) to index (0-4)
	SelectSlot(SlotNumber - 1);
}

void UHotbarWidget::UseCurrentSlot()
{
	OnSlotUsed.Broadcast(CurrentSlotIndex);
	OnUseSlot(CurrentSlotIndex);

	// Get the spell for this slot
	TSubclassOf<ASpellBase> SpellClass = GetSpellInSlot(CurrentSlotIndex);
	if (!SpellClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("HotbarWidget: No spell in slot %d"), CurrentSlotIndex + 1);
		return;
	}

	// Get the wizard character
	AWizardPlayerController* PC = Cast<AWizardPlayerController>(GetOwningPlayer());
	if (!PC)
	{
		return;
	}

	APawn* Pawn = PC->GetPawn();
	AWizardCharacter* Wizard = Cast<AWizardCharacter>(Pawn);
	if (!Wizard)
	{
		return;
	}

	// Find or spawn the spell actor
	ASpellBase* Spell = nullptr;
	for (TActorIterator<ASpellBase> It(GetWorld(), SpellClass); It; ++It)
	{
		Spell = *It;
		break;
	}

	if (!Spell)
	{
		// Spawn spell if it doesn't exist
		Spell = GetWorld()->SpawnActor<ASpellBase>(SpellClass);
	}

	if (Spell && Spell->CanCast())
	{
		Spell->Execute(Wizard);
		UE_LOG(LogTemp, Log, TEXT("HotbarWidget: Cast %s from slot %d"), *Spell->SpellName, CurrentSlotIndex + 1);
	}
	else if (Spell)
	{
		UE_LOG(LogTemp, Warning, TEXT("HotbarWidget: %s on cooldown (%.1fs remaining)"), *Spell->SpellName, Spell->GetCooldownRemaining());
	}
}

TSubclassOf<ASpellBase> UHotbarWidget::GetSpellInSlot(int32 SlotIndex) const
{
	if (HotbarSpells.IsValidIndex(SlotIndex))
	{
		return HotbarSpells[SlotIndex];
	}

	return nullptr;
}

void UHotbarWidget::UpdateVisuals()
{
	// Update each slot's border color based on selection
	for (int32 i = 0; i < NumSlots; i++)
	{
		UBorder* SlotBorder = GetSlotBorder(i);
		if (SlotBorder)
		{
			FLinearColor Color = (i == CurrentSlotIndex) ? SelectedColor : UnselectedColor;
			SlotBorder->SetBrushColor(Color);
		}
	}
}

UBorder* UHotbarWidget::GetSlotBorder(int32 SlotIndex) const
{
	switch (SlotIndex)
	{
		case 0: return HotbarSlot0;
		case 1: return HotbarSlot1;
		case 2: return HotbarSlot2;
		case 3: return HotbarSlot3;
		case 4: return HotbarSlot4;
		default: return nullptr;
	}
}

int32 UHotbarWidget::ClampSlotIndex(int32 Index) const
{
	// Wrap around: if index goes below 0, go to last slot
	// If index goes above max, go to first slot
	if (Index < 0)
	{
		return NumSlots - 1;
	}
	else if (Index >= NumSlots)
	{
		return 0;
	}

	return Index;
}

