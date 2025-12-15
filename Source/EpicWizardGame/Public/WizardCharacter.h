// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WizardCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class UAnimMontage;
class UAnimSequenceBase;
class UAnimInstance;
class USkeletalMeshComponent;
class USpringArmComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWizardHealthChangedSignature, float, CurrentHP, float, MaxHP);

UCLASS()
class EPICWIZARDGAME_API AWizardCharacter : public ACharacter
{
	GENERATED_BODY()

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UCameraComponent* FirstPersonCamera;

	/** Top-down spring arm for overhead camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USpringArmComponent* TopDownSpringArm;

	/** Top-down camera (overhead) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UCameraComponent* TopDownCamera;

	/** Editable pitch for the top-down camera arm (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(AllowPrivateAccess="true"))
	float TopCameraAngle = -65.0f;

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

	/** Spell cast animation sequence (fallback to use when no montage is provided) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimSequenceBase* SpellCastAnimation;

	/** Play rate for spell cast sequence */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations", meta=(ClampMin="0.01", UIMin="0.1", UIMax="3.0"))
	float SpellCastAnimPlayRate = 1.0f;

	/** Walk loop animation (played when moving) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimSequenceBase* WalkAnimation;

	/** Play rate for the walk loop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations", meta=(ClampMin="0.01", UIMin="0.1", UIMax="3.0"))
	float WalkAnimPlayRate = 1.0f;

	/** Minimum speed to count as walking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations", meta=(ClampMin="0.0"))
	float WalkVelocityThreshold = 10.0f;

	/** Max HP for the wizard */
	UPROPERTY(EditAnywhere, Category="Health")
	float MaxHP = 100.0f;

	/** Current HP */
	UPROPERTY(BlueprintReadOnly, Category="Health")
	float CurrentHP = 0.0f;

	/** True if currently casting a spell (prevents animation cancelling) */
	bool bIsCasting = false;

	/** True when using the top-down view (courtyard/rampart levels) */
	bool bIsTopDownViewActive = false;

public:

	/** Fired when the wizard's health changes */
	UPROPERTY(BlueprintAssignable, Category="Wizard|Health")
	FWizardHealthChangedSignature OnHealthChanged;

	AWizardCharacter();

protected:

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;

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

	/** Returns current HP percentage (0-1) */
	UFUNCTION(BlueprintCallable, Category="Wizard")
	float GetHealthPercent() const { return MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f; }

	/** Returns current HP */
	UFUNCTION(BlueprintCallable, Category="Wizard")
	float GetCurrentHP() const { return CurrentHP; }

	/** Returns max HP */
	UFUNCTION(BlueprintCallable, Category="Wizard")
	float GetMaxHP() const { return MaxHP; }

	/** Calculates the current aim origin/direction (handles top-down cursor aiming) */
	bool GetAimData(FVector& OutOrigin, FVector& OutDirection) const;

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

	/** Called when cast sequence ends */
	void OnCastAnimationFinished();

	/** Called when HP is depleted */
	void Die();

	/** Switches cameras if we’re in a top-down level */
	void TryActivateTopDownView();

	/** Returns true if the current map should use the top-down camera */
	bool ShouldUseTopDownCamera() const;

	/** Blueprint event for spell cast effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Wizard", meta=(DisplayName="On Spell Cast"))
	void BP_OnSpellCast();

	/** Blueprint event for death */
	UFUNCTION(BlueprintImplementableEvent, Category="Wizard", meta=(DisplayName="On Death"))
	void BP_OnDeath();

private:
	void UpdateMovementAnimation();

	UPROPERTY(Transient)
	UAnimMontage* WalkDynamicMontage = nullptr;

	UPROPERTY(Transient)
	bool bUsingSingleNodeWalk = false;

	UPROPERTY(Transient)
	TWeakObjectPtr<USkeletalMeshComponent> WalkSingleNodeMesh;

	UPROPERTY(Transient)
	TSubclassOf<UAnimInstance> SavedWalkAnimClass;

	UPROPERTY(Transient)
	TEnumAsByte<EAnimationMode::Type> SavedWalkAnimMode;

	UPROPERTY(Transient)
	bool bUsingCastSingleNode = false;

	UPROPERTY(Transient)
	TWeakObjectPtr<USkeletalMeshComponent> CastSingleNodeMesh;

	UPROPERTY(Transient)
	TSubclassOf<UAnimInstance> SavedCastAnimClass;

	UPROPERTY(Transient)
	TEnumAsByte<EAnimationMode::Type> SavedCastAnimMode;

	UPROPERTY(Transient)
	FTimerHandle CastAnimationTimer;
};
