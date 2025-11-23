// Fill out your copyright notice in the Description page of Project Settings.

#include "HotbarWidget.h"
#include "Components/Border.h"

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

	UE_LOG(LogTemp, Log, TEXT("HotbarWidget: Used slot %d"), CurrentSlotIndex + 1);
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

