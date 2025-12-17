// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Turret.generated.h"

class AZombieCharacter;
class ASpellProjectile;
class USceneComponent;
class UBoxComponent;
class UWidgetComponent;
class UUserWidget;

UCLASS()
class EPICWIZARDGAME_API ATurret : public APawn
{
	GENERATED_BODY()

protected:

	/** Root scene component (keeps attachment stable for BP components) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USceneComponent* SceneRoot;

	/** Floating health bar widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UWidgetComponent* HealthBarWidget;

	/** Collision box for zombie attacks (query-only overlap) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UBoxComponent* AttackCollision;

	/** Projectile class to spawn (use BP_Projectile - the fireball) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	TSubclassOf<ASpellProjectile> ProjectileClass;

	/** Detection range for finding zombies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float DetectionRange = 2000.0f;

	/** Time between shots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float FireRate = 1.0f;

	/** Damage dealt by projectiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float ProjectileDamage = 25.0f;

	/** Vertical offset so spawned projectiles start higher on the turret */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float ProjectileVerticalOffset = 50.0f;

	/** Pitch bias (degrees) applied to the projectile's actual travel direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float ProjectileAimPitchOffset = 0.0f;

	/** Visual-only pitch adjustment for spawned projectiles (aim remains unchanged) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float ProjectileVisualPitchOffset = 0.0f;

	/** Timer for firing */
	float FireTimer = 0.0f;

	/** Max HP for the turret */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
	float MaxHP = 100.0f;

	/** Current HP */
	UPROPERTY(BlueprintReadOnly, Category="Health")
	float CurrentHP = 0.0f;

	/** True if turret is destroyed */
	UPROPERTY(BlueprintReadOnly, Category="Health")
	bool bIsDestroyed = false;

	/** True if this turret is a placement preview (should not be targetable) */
	UPROPERTY(BlueprintReadOnly, Category="Turret")
	bool bIsPreviewTurret = false;

public:
	// Sets default values for this pawn's properties
	ATurret();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Handle damage to the turret */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Returns current HP percentage (0-1) */
	UFUNCTION(BlueprintCallable, Category="Turret|Health")
	float GetHealthPercent() const { return MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f; }

	/** Returns current HP */
	UFUNCTION(BlueprintCallable, Category="Turret|Health")
	float GetCurrentHP() const { return CurrentHP; }

	/** Returns max HP */
	UFUNCTION(BlueprintCallable, Category="Turret|Health")
	float GetMaxHP() const { return MaxHP; }

	/** Returns true if turret is destroyed */
	UFUNCTION(BlueprintCallable, Category="Turret")
	bool IsDestroyed() const { return bIsDestroyed; }

	/** Returns true if turret is a placement preview */
	UFUNCTION(BlueprintCallable, Category="Turret")
	bool IsPreviewTurret() const { return bIsPreviewTurret; }

	/** Mark turret as a placement preview (hides health bar, makes it untargetable) */
	UFUNCTION(BlueprintCallable, Category="Turret")
	void SetIsPreviewTurret(bool bIsPreview);

protected:

	/** Find the nearest zombie within detection range */
	virtual AZombieCharacter* FindNearestZombie();

	/** Shoot projectile at target */
	virtual void ShootAtTarget(AZombieCharacter* Target);

	/** Called when turret HP is depleted */
	void DestroyTurret();

	/** Blueprint event for taking damage */
	UFUNCTION(BlueprintImplementableEvent, Category="Turret", meta=(DisplayName="On Turret Damaged"))
	void BP_OnTurretDamaged(float DamageTaken, float RemainingHP);

	/** Blueprint event for turret destruction */
	UFUNCTION(BlueprintImplementableEvent, Category="Turret", meta=(DisplayName="On Turret Destroyed"))
	void BP_OnTurretDestroyed();

};
