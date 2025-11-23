// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HotbarWidget.generated.h"

class UBorder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHotbarSlotChanged, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHotbarSlotUsed, int32, SlotIndex);

/**
 * Base widget class for the hotbar system
 * Handles slot selection via scroll wheel and number keys
 */
UCLASS()
class EPICWIZARDGAME_API UHotbarWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Number of hotbar slots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hotbar")
	int32 NumSlots = 5;

	/** Currently selected hotbar slot (0-4 for slots 1-5) */
	UPROPERTY(BlueprintReadOnly, Category="Hotbar")
	int32 CurrentSlotIndex = 0;

	/** Color for selected slot border */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hotbar|Visuals")
	FLinearColor SelectedColor = FLinearColor::Yellow;

	/** Color for unselected slot border */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hotbar|Visuals")
	FLinearColor UnselectedColor = FLinearColor::White;

	/** Called when the selected slot changes */
	UPROPERTY(BlueprintAssignable, Category="Hotbar")
	FOnHotbarSlotChanged OnSlotChanged;

	/** Called when a hotbar slot is used */
	UPROPERTY(BlueprintAssignable, Category="Hotbar")
	FOnHotbarSlotUsed OnSlotUsed;

	/** References to hotbar slot borders (assign in Blueprint) */
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category="Hotbar")
	UBorder* HotbarSlot0;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category="Hotbar")
	UBorder* HotbarSlot1;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category="Hotbar")
	UBorder* HotbarSlot2;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category="Hotbar")
	UBorder* HotbarSlot3;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category="Hotbar")
	UBorder* HotbarSlot4;

	/** Select next hotbar slot (scroll down) */
	UFUNCTION(BlueprintCallable, Category="Hotbar")
	void SelectNextSlot();

	/** Select previous hotbar slot (scroll up) */
	UFUNCTION(BlueprintCallable, Category="Hotbar")
	void SelectPreviousSlot();

	/** Select specific hotbar slot by index (0-4) */
	UFUNCTION(BlueprintCallable, Category="Hotbar")
	void SelectSlot(int32 SlotIndex);

	/** Select slot by number key (1-5) */
	UFUNCTION(BlueprintCallable, Category="Hotbar")
	void SelectSlotByNumber(int32 SlotNumber);

	/** Use the currently selected hotbar slot */
	UFUNCTION(BlueprintCallable, Category="Hotbar")
	void UseCurrentSlot();

	/** Get the currently selected slot index */
	UFUNCTION(BlueprintPure, Category="Hotbar")
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	/** Blueprint event called when a slot should be used - implement in BP */
	UFUNCTION(BlueprintImplementableEvent, Category="Hotbar")
	void OnUseSlot(int32 SlotIndex);

protected:

	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Update visual appearance of all slots */
	UFUNCTION(BlueprintCallable, Category="Hotbar")
	void UpdateVisuals();

	/** Get border widget for a given slot index */
	UBorder* GetSlotBorder(int32 SlotIndex) const;

private:

	/** Clamp slot index to valid range */
	int32 ClampSlotIndex(int32 Index) const;
};
