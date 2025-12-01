// Fill out your copyright notice in the Description page of Project Settings.

#include "HotbarWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "SpellBase.h"
#include "Turret.h"
#include "WizardCharacter.h"
#include "WizardPlayerController.h"
#include "WaveManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

void UHotbarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Start with first slot selected
	CurrentSlotIndex = 0;
	CurrentMode = EHotbarMode::Spells;
	PreviewTurret = nullptr;
	UpdateVisuals();
	UpdateSlotTextures();
}

void UHotbarWidget::NativeDestruct()
{
	DestroyPreviewTurret();
	Super::NativeDestruct();
}

void UHotbarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update preview turret if in turret mode and not on slot 4
	if (CurrentMode == EHotbarMode::Turrets && CurrentSlotIndex != 4)
	{
		UpdatePreviewTurret();
	}
	else
	{
		// Hide or destroy preview when not in turret placement mode
		if (PreviewTurret)
		{
			PreviewTurret->SetActorHiddenInGame(true);
		}
	}
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

	// If slot 4 is selected, toggle mode
	if (CurrentSlotIndex == 4)
	{
		ToggleMode();
		return;
	}

	// Handle spell mode
	if (CurrentMode == EHotbarMode::Spells)
	{
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
	// Handle turret mode
	else if (CurrentMode == EHotbarMode::Turrets)
	{
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

		// Perform raycast from camera to find placement location
		FVector CameraLocation;
		FRotator CameraRotation;
		PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

		FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 5000.0f);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Wizard);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			CameraLocation,
			TraceEnd,
			ECC_Visibility,
			QueryParams
		);

		if (bHit)
		{
			PlaceTurret(CurrentSlotIndex, HitResult.Location);
			UE_LOG(LogTemp, Log, TEXT("HotbarWidget: Placed turret from slot %d at location %s"), CurrentSlotIndex, *HitResult.Location.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HotbarWidget: No valid placement location found for turret"));
		}
	}
}

void UHotbarWidget::UseSlot(int32 SlotIndex)
{
	// Validate slot index
	if (SlotIndex < 0 || SlotIndex >= NumSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("HotbarWidget: Invalid slot index %d"), SlotIndex);
		return;
	}

	// If slot 4 is clicked, toggle mode
	if (SlotIndex == 4)
	{
		ToggleMode();
		return;
	}

	// Handle spell mode (slots 0-3)
	if (CurrentMode == EHotbarMode::Spells)
	{
		// Select and use the slot for spell casting
		SelectSlot(SlotIndex);
		UseCurrentSlot();
	}
	// Handle turret mode (slots 0-3)
	else if (CurrentMode == EHotbarMode::Turrets)
	{
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

		// Perform raycast from camera to find placement location
		FVector CameraLocation;
		FRotator CameraRotation;
		PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

		FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 5000.0f);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Wizard);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			CameraLocation,
			TraceEnd,
			ECC_Visibility,
			QueryParams
		);

		if (bHit)
		{
			PlaceTurret(SlotIndex, HitResult.Location);
			UE_LOG(LogTemp, Log, TEXT("HotbarWidget: Placed turret from slot %d at location %s"), SlotIndex, *HitResult.Location.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HotbarWidget: No valid placement location found for turret"));
		}
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

TSubclassOf<ATurret> UHotbarWidget::GetTurretInSlot(int32 SlotIndex) const
{
	if (HotbarTurrets.IsValidIndex(SlotIndex))
	{
		return HotbarTurrets[SlotIndex];
	}

	return nullptr;
}

void UHotbarWidget::ToggleMode()
{
	if (CurrentMode == EHotbarMode::Spells)
	{
		SetMode(EHotbarMode::Turrets);
	}
	else
	{
		SetMode(EHotbarMode::Spells);
	}
}

void UHotbarWidget::SetMode(EHotbarMode NewMode)
{
	if (CurrentMode != NewMode)
	{
		CurrentMode = NewMode;
		UpdateSlotTextures();
		OnModeChanged.Broadcast(CurrentMode);

		// Destroy preview when switching to spell mode
		if (CurrentMode == EHotbarMode::Spells)
		{
			DestroyPreviewTurret();
		}

		UE_LOG(LogTemp, Log, TEXT("HotbarWidget: Switched to %s mode"),
			CurrentMode == EHotbarMode::Spells ? TEXT("Spell") : TEXT("Turret"));
	}
}

void UHotbarWidget::PlaceTurret(int32 SlotIndex, const FVector& Location)
{
	TSubclassOf<ATurret> TurretClass = GetTurretInSlot(SlotIndex);
	if (!TurretClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("HotbarWidget: No turret in slot %d"), SlotIndex);
		return;
	}

	// Check if player can afford this turret
	if (!CanAffordTurret(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("HotbarWidget: Cannot afford turret! Cost: $%d"), GetTurretCost(SlotIndex));
		return;
	}

	// Find wave manager and spend money
	AWaveManager* WaveManager = nullptr;
	for (TActorIterator<AWaveManager> It(GetWorld()); It; ++It)
	{
		WaveManager = *It;
		break;
	}

	if (!WaveManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("HotbarWidget: No wave manager found!"));
		return;
	}

	int32 TurretCost = GetTurretCost(SlotIndex);
	if (!WaveManager->SpendMoney(TurretCost))
	{
		return; // Failed to spend money
	}

	// Spawn the turret
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FRotator SpawnRotation = FRotator::ZeroRotator;
	ATurret* Turret = GetWorld()->SpawnActor<ATurret>(TurretClass, Location, SpawnRotation, SpawnParams);

	if (Turret)
	{
		UE_LOG(LogTemp, Log, TEXT("HotbarWidget: Spawned turret for $%d (Remaining: $%d)"),
			TurretCost, WaveManager->GetPlayerMoney());
	}
}

void UHotbarWidget::UpdateSlotTextures()
{
	if (CurrentMode == EHotbarMode::Spells)
	{
		// Set slots 0-3 to spell textures
		for (int32 i = 0; i < 4; i++)
		{
			UImage* SlotImage = GetSlotImage(i);
			if (SlotImage && SpellTextures.IsValidIndex(i) && SpellTextures[i])
			{
				SlotImage->SetBrushFromTexture(SpellTextures[i]);
			}
		}

		// Set slot 4 to base turret texture
		UImage* Slot4Image = GetSlotImage(4);
		if (Slot4Image && BaseTurretTexture)
		{
			Slot4Image->SetBrushFromTexture(BaseTurretTexture);
		}
	}
	else // Turret mode
	{
		// Set slots 0-3 to turret textures
		for (int32 i = 0; i < 4; i++)
		{
			UImage* SlotImage = GetSlotImage(i);
			if (SlotImage && TurretTextures.IsValidIndex(i) && TurretTextures[i])
			{
				SlotImage->SetBrushFromTexture(TurretTextures[i]);
			}
		}

		// Set slot 4 to staff texture
		UImage* Slot4Image = GetSlotImage(4);
		if (Slot4Image && StaffTexture)
		{
			Slot4Image->SetBrushFromTexture(StaffTexture);
		}
	}
}

UImage* UHotbarWidget::GetSlotImage(int32 SlotIndex) const
{
	switch (SlotIndex)
	{
		case 0: return ImageSlot0;
		case 1: return ImageSlot1;
		case 2: return ImageSlot2;
		case 3: return ImageSlot3;
		case 4: return ImageSlot4;
		default: return nullptr;
	}
}

void UHotbarWidget::UpdatePreviewTurret()
{
	// Get the wizard player controller
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

	// Perform raycast from camera to find placement location
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 5000.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Wizard);
	if (PreviewTurret)
	{
		QueryParams.AddIgnoredActor(PreviewTurret);
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CameraLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (bHit)
	{
		// Spawn preview turret if it doesn't exist or is wrong type
		TSubclassOf<ATurret> DesiredTurretClass = GetTurretInSlot(CurrentSlotIndex);
		if (DesiredTurretClass && (!PreviewTurret || PreviewTurret->GetClass() != DesiredTurretClass))
		{
			SpawnPreviewTurret();
		}

		// Update preview turret position
		if (PreviewTurret)
		{
			PreviewTurret->SetActorLocation(HitResult.Location);
			PreviewTurret->SetActorHiddenInGame(false);
		}
	}
	else
	{
		// Hide preview if no valid placement
		if (PreviewTurret)
		{
			PreviewTurret->SetActorHiddenInGame(true);
		}
	}
}

void UHotbarWidget::SpawnPreviewTurret()
{
	// Destroy existing preview
	DestroyPreviewTurret();

	TSubclassOf<ATurret> TurretClass = GetTurretInSlot(CurrentSlotIndex);
	if (!TurretClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	PreviewTurret = GetWorld()->SpawnActor<ATurret>(TurretClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (PreviewTurret)
	{
		// Disable collision and tick on preview
		PreviewTurret->SetActorEnableCollision(false);
		PreviewTurret->SetActorTickEnabled(false);

		// Make the preview translucent
		TArray<UActorComponent*> Components;
		PreviewTurret->GetComponents(UStaticMeshComponent::StaticClass(), Components);

		for (UActorComponent* Component : Components)
		{
			UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Component);
			if (MeshComp)
			{
				// Apply preview material if set, otherwise just adjust opacity
				if (PreviewMaterial)
				{
					for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
					{
						MeshComp->SetMaterial(i, PreviewMaterial);
					}
				}
				else
				{
					// Create a dynamic material instance with transparency
					for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
					{
						UMaterialInterface* OriginalMat = MeshComp->GetMaterial(i);
						if (OriginalMat)
						{
							UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(OriginalMat, this);
							if (DynMat)
							{
								DynMat->SetScalarParameterValue(FName("Opacity"), PreviewOpacity);
								MeshComp->SetMaterial(i, DynMat);
							}
						}
					}
				}

				// Enable transparency rendering
				MeshComp->SetRenderCustomDepth(false);
			}
		}

		PreviewTurret->SetActorHiddenInGame(true);
		UE_LOG(LogTemp, Log, TEXT("HotbarWidget: Spawned preview turret"));
	}
}

void UHotbarWidget::DestroyPreviewTurret()
{
	if (PreviewTurret)
	{
		PreviewTurret->Destroy();
		PreviewTurret = nullptr;
		UE_LOG(LogTemp, Log, TEXT("HotbarWidget: Destroyed preview turret"));
	}
}

int32 UHotbarWidget::GetTurretCost(int32 SlotIndex) const
{
	if (TurretCosts.IsValidIndex(SlotIndex))
	{
		return TurretCosts[SlotIndex];
	}

	// Default costs if not set: Ice=800, Fire=1000, Lightning=1500, Air=2000
	switch (SlotIndex)
	{
		case 0: return 1000; // Fire
		case 1: return 800;  // Ice
		case 2: return 1500; // Lightning
		case 3: return 2000; // Air
		default: return 0;
	}
}

bool UHotbarWidget::CanAffordTurret(int32 SlotIndex) const
{
	// Find wave manager
	AWaveManager* WaveManager = nullptr;
	for (TActorIterator<AWaveManager> It(GetWorld()); It; ++It)
	{
		WaveManager = *It;
		break;
	}

	if (!WaveManager)
	{
		return false;
	}

	int32 TurretCost = GetTurretCost(SlotIndex);
	return WaveManager->GetPlayerMoney() >= TurretCost;
}

