// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpellProjectile.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class EPICWIZARDGAME_API ASpellProjectile : public AActor
{
	GENERATED_BODY()

protected:

	/** Collision sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USphereComponent* CollisionSphere;

	/** Visual mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* MeshComponent;

	/** Projectile movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UProjectileMovementComponent* ProjectileMovement;

public:

	ASpellProjectile();

	/** Damage this projectile deals */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile")
	float Damage = 25.0f;

	/** Lifetime before auto-destroying */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile")
	float Lifetime = 5.0f;

	/** If true, pawn collisions overlap instead of block so projectile can pierce */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile")
	bool bPierceTargets = false;

	/** Optional freeze effect applied on hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Freeze")
	bool bApplyFreeze = false;

	/** Duration of freeze/slow in seconds (used if bApplyFreeze is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Freeze", meta=(EditCondition="bApplyFreeze", ClampMin="0.0"))
	float FreezeDuration = 0.0f;

	/** Movement speed multiplier while frozen (used if bApplyFreeze is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Freeze", meta=(EditCondition="bApplyFreeze", ClampMin="0.0"))
	float FreezeSpeedMultiplier = 1.0f;

	/** Initialize the projectile with direction and damage */
	UFUNCTION(BlueprintCallable, Category="Projectile")
	void InitializeProjectile(const FVector& Direction, float InDamage);

protected:

	virtual void BeginPlay() override;

	/** Called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Called when projectile overlaps something (used for piercing) */
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Track already hit actors when piercing to avoid repeat hits */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> PiercedActors;
};
