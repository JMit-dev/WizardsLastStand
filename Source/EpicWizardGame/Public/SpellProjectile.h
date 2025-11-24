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

	/** Initialize the projectile with direction and damage */
	UFUNCTION(BlueprintCallable, Category="Projectile")
	void InitializeProjectile(const FVector& Direction, float InDamage);

protected:

	virtual void BeginPlay() override;

	/** Called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
