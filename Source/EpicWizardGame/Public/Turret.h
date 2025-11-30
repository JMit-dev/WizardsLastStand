// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Turret.generated.h"

class AZombieCharacter;
class ASpellProjectile;

UCLASS()
class EPICWIZARDGAME_API ATurret : public APawn
{
	GENERATED_BODY()

protected:

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

	/** Timer for firing */
	float FireTimer = 0.0f;

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

protected:

	/** Find the nearest zombie within detection range */
	virtual AZombieCharacter* FindNearestZombie();

	/** Shoot projectile at target */
	virtual void ShootAtTarget(AZombieCharacter* Target);

};
