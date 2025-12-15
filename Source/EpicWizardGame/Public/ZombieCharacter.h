// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieCharacter.generated.h"

class UAnimMontage;
class UAnimSequenceBase;
class UAnimInstance;
class USkeletalMeshComponent;
class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FZombieDeathDelegate);

UCLASS()
class EPICWIZARDGAME_API AZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	/** Max HP for the zombie */
	UPROPERTY(BlueprintReadWrite, Category="Health")
	float MaxHP = 100.0f;

	/** Current HP */
	UPROPERTY(BlueprintReadWrite, Category="Health")
	float CurrentHP = 0.0f;

protected:

	/** Floating health bar widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UWidgetComponent* HealthBarWidget;

	/** Attack animation montage */
	UPROPERTY(EditAnywhere, Category="Animations")
	UAnimMontage* AttackMontage;

	/** Walk loop animation (played when moving) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimSequenceBase* WalkAnimation;

	/** Play rate for the walk loop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations", meta=(ClampMin="0.01", UIMin="0.1", UIMax="3.0"))
	float WalkAnimPlayRate = 1.0f;

	/** Minimum speed to count as walking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations", meta=(ClampMin="0.0"))
	float WalkVelocityThreshold = 10.0f;

	/** Damage dealt per attack (33.33 for 3-hit kill with 100HP) */
	UPROPERTY(EditAnywhere, Category="Combat")
	float AttackDamage = 33.33f;

	/** Range for attack to hit */
	UPROPERTY(EditAnywhere, Category="Combat")
	float AttackRange = 150.0f;

	/** Range for attacking towers (can be larger since towers are bigger) */
	UPROPERTY(EditAnywhere, Category="Combat")
	float TowerAttackRange = 1000.0f;

	/** Time to wait after death before destroying */
	UPROPERTY(EditAnywhere, Category="Death")
	float DeathDestroyDelay = 5.0f;

	/** True if currently attacking */
	bool bIsAttacking = false;

	/** True if dead */
	bool bIsDead = false;

	/** Timer for deferred destruction */
	FTimerHandle DeathTimer;

public:

	/** Delegate broadcast when zombie dies */
	FZombieDeathDelegate OnZombieDeath;

public:

	AZombieCharacter();

protected:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaTime) override;

public:

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Perform attack - called by AI controller */
	UFUNCTION(BlueprintCallable, Category="Zombie")
	void DoAttack();

	/** Returns true if currently attacking */
	UFUNCTION(BlueprintCallable, Category="Zombie")
	bool IsAttacking() const { return bIsAttacking; }

	/** Returns true if dead */
	UFUNCTION(BlueprintCallable, Category="Zombie")
	bool IsDead() const { return bIsDead; }

	/** Returns current HP percentage (0-1) */
	UFUNCTION(BlueprintCallable, Category="Zombie")
	float GetHealthPercent() const { return MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f; }

protected:

	/** Called when attack montage ends */
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** Apply damage to target in range */
	void ApplyAttackDamage();

	/** Called when HP is depleted */
	void Die();

	/** Called after death delay to destroy actor */
	void DeferredDestruction();

	/** Blueprint event for attack hit */
	UFUNCTION(BlueprintImplementableEvent, Category="Zombie", meta=(DisplayName="On Attack"))
	void BP_OnAttack();

	/** Blueprint event for death */
	UFUNCTION(BlueprintImplementableEvent, Category="Zombie", meta=(DisplayName="On Death"))
	void BP_OnDeath();

private:
	void UpdateMovementAnimation();

	UPROPERTY(Transient)
	bool bUsingSingleNodeWalk = false;

	UPROPERTY(Transient)
	TWeakObjectPtr<USkeletalMeshComponent> WalkSingleNodeMesh;

	UPROPERTY(Transient)
	TSubclassOf<UAnimInstance> SavedWalkAnimClass;

	UPROPERTY(Transient)
	TEnumAsByte<EAnimationMode::Type> SavedWalkAnimMode;
};
