// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WizardCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class UAnimMontage;

UCLASS()
class EPICWIZARDGAME_API AWizardCharacter : public ACharacter
{
	GENERATED_BODY()

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UCameraComponent* FirstPersonCamera;

	/** First person arms mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** Staff mesh attached to right hand */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* StaffMesh;

protected:

	/** Input mapping context */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* DefaultMappingContext;

	/** Move input action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look input action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Jump input action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Cast spell input action (left click) */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* CastSpellAction;

	/** Hotbar scroll input action (mouse wheel) */
	UPROPERTY(EditAnywhere, Category="Input|Hotbar")
	UInputAction* HotbarScrollAction;

	/** Hotbar slot 1 input action */
	UPROPERTY(EditAnywhere, Category="Input|Hotbar")
	UInputAction* HotbarSlot1Action;

	/** Hotbar slot 2 input action */
	UPROPERTY(EditAnywhere, Category="Input|Hotbar")
	UInputAction* HotbarSlot2Action;

	/** Hotbar slot 3 input action */
	UPROPERTY(EditAnywhere, Category="Input|Hotbar")
	UInputAction* HotbarSlot3Action;

	/** Hotbar slot 4 input action */
	UPROPERTY(EditAnywhere, Category="Input|Hotbar")
	UInputAction* HotbarSlot4Action;

	/** Hotbar slot 5 input action */
	UPROPERTY(EditAnywhere, Category="Input|Hotbar")
	UInputAction* HotbarSlot5Action;

	/** Socket name on the mesh for staff attachment */
	UPROPERTY(EditAnywhere, Category="Staff")
	FName StaffSocketName = FName("hand_r");

	/** Spell cast animation montage */
	UPROPERTY(EditAnywhere, Category="Animations")
	UAnimMontage* SpellCastMontage;

	/** Max HP for the wizard */
	UPROPERTY(EditAnywhere, Category="Health")
	float MaxHP = 100.0f;

	/** Current HP */
	UPROPERTY(BlueprintReadOnly, Category="Health")
	float CurrentHP = 0.0f;

	/** True if currently casting a spell (prevents animation cancelling) */
	bool bIsCasting = false;

public:

	AWizardCharacter();

protected:

	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Cast spell - bound to left click */
	UFUNCTION(BlueprintCallable, Category="Wizard")
	void DoCastSpell();

	/** Returns true if currently casting */
	UFUNCTION(BlueprintCallable, Category="Wizard")
	bool IsCasting() const { return bIsCasting; }

	/** Returns the first person camera */
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	/** Returns the first person mesh */
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns the staff mesh */
	UStaticMeshComponent* GetStaffMesh() const { return StaffMesh; }

protected:

	/** Movement input handler */
	void MoveInput(const struct FInputActionValue& Value);

	/** Look input handler */
	void LookInput(const struct FInputActionValue& Value);

	/** Hotbar scroll input handler */
	void HotbarScrollInput(const struct FInputActionValue& Value);

	/** Hotbar slot selection handlers */
	void SelectHotbarSlot1();
	void SelectHotbarSlot2();
	void SelectHotbarSlot3();
	void SelectHotbarSlot4();
	void SelectHotbarSlot5();

	/** Called when cast montage ends */
	UFUNCTION()
	void OnCastMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** Called when HP is depleted */
	void Die();

	/** Blueprint event for spell cast effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Wizard", meta=(DisplayName="On Spell Cast"))
	void BP_OnSpellCast();

	/** Blueprint event for death */
	UFUNCTION(BlueprintImplementableEvent, Category="Wizard", meta=(DisplayName="On Death"))
	void BP_OnDeath();
};
