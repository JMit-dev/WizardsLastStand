// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HotbarWidget.generated.h"

class UBorder;
class UImage;
class UTexture2D;

UENUM(BlueprintType)
enum class EHotbarMode : uint8
{
	Spells UMETA(DisplayName = "Spells"),
	Turrets UMETA(DisplayName = "Turrets")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHotbarSlotChanged, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHotbarSlotUsed, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHotbarModeChanged, EHotbarMode, NewMode);

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

	/** Called when hotbar mode changes */
	UPROPERTY(BlueprintAssignable, Category="Hotbar")
	FOnHotbarModeChanged OnModeChanged;

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

	/** References to hotbar slot images (assign in Blueprint) */
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category="Hotbar")
	UImage* ImageSlot0;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category="Hotbar")
	UImage* ImageSlot1;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category="Hotbar")
	UImage* ImageSlot2;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category="Hotbar")
	UImage* ImageSlot3;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget), Category="Hotbar")
	UImage* ImageSlot4;

	/** Current hotbar mode (spells or turrets) */
	UPROPERTY(BlueprintReadOnly, Category="Hotbar")
	EHotbarMode CurrentMode = EHotbarMode::Spells;

	/** Spell assigned to each hotbar slot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hotbar|Spells")
	TArray<TSubclassOf<class ASpellBase>> HotbarSpells;

	/** Turret classes for each slot (Fire, Ice, Lightning, Air) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hotbar|Turrets")
	TArray<TSubclassOf<class ATurret>> HotbarTurrets;

	/** Spell textures for slots 0-3 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hotbar|Textures")
	TArray<UTexture2D*> SpellTextures;

	/** Turret textures for slots 0-3 (Fire, Ice, Lightning, Air) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hotbar|Textures")
	TArray<UTexture2D*> TurretTextures;

	/** Base turret texture for slot 4 in spell mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hotbar|Textures")
	UTexture2D* BaseTurretTexture;

	/** Staff texture for slot 4 in turret mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hotbar|Textures")
	UTexture2D* StaffTexture;

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

	/** Use a specific hotbar slot (handles mode toggle and turret placement automatically) */
	UFUNCTION(BlueprintCallable, Category="Hotbar")
	void UseSlot(int32 SlotIndex);

	/** Get the currently selected slot index */
	UFUNCTION(BlueprintPure, Category="Hotbar")
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	/** Get the spell in a specific slot */
	UFUNCTION(BlueprintPure, Category="Hotbar")
	TSubclassOf<class ASpellBase> GetSpellInSlot(int32 SlotIndex) const;

	/** Get the turret class in a specific slot */
	UFUNCTION(BlueprintPure, Category="Hotbar")
	TSubclassOf<class ATurret> GetTurretInSlot(int32 SlotIndex) const;

	/** Toggle between spell and turret mode */
	UFUNCTION(BlueprintCallable, Category="Hotbar")
	void ToggleMode();

	/** Set hotbar mode (spells or turrets) */
	UFUNCTION(BlueprintCallable, Category="Hotbar")
	void SetMode(EHotbarMode NewMode);

	/** Get current hotbar mode */
	UFUNCTION(BlueprintPure, Category="Hotbar")
	EHotbarMode GetCurrentMode() const { return CurrentMode; }

	/** Place turret at world location */
	UFUNCTION(BlueprintCallable, Category="Hotbar")
	void PlaceTurret(int32 SlotIndex, const FVector& Location);

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

	/** Update slot textures based on current mode */
	UFUNCTION(BlueprintCallable, Category="Hotbar")
	void UpdateSlotTextures();

	/** Get border widget for a given slot index */
	UBorder* GetSlotBorder(int32 SlotIndex) const;

	/** Get image widget for a given slot index */
	UImage* GetSlotImage(int32 SlotIndex) const;

private:

	/** Clamp slot index to valid range */
	int32 ClampSlotIndex(int32 Index) const;
};
