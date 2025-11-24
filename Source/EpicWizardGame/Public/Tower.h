// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tower.generated.h"

class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTowerDestroyedDelegate);

UCLASS()
class EPICWIZARDGAME_API ATower : public AActor
{
	GENERATED_BODY()

protected:

	/** Floating health bar widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UWidgetComponent* HealthBarWidget;

	/** Max HP for the tower */
	UPROPERTY(EditAnywhere, Category="Health")
	float MaxHP = 1000.0f;

	/** Current HP */
	UPROPERTY(BlueprintReadOnly, Category="Health")
	float CurrentHP = 0.0f;

	/** True if tower is destroyed */
	bool bIsDestroyed = false;

public:

	/** Delegate broadcast when tower is destroyed */
	FTowerDestroyedDelegate OnTowerDestroyed;

public:
	// Sets default values for this actor's properties
	ATower();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// The mesh that will show in the world
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* TowerMesh;

	/** Handle damage to the tower */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Returns current HP percentage (0-1) */
	UFUNCTION(BlueprintCallable, Category="Tower")
	float GetHealthPercent() const { return MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f; }

	/** Returns true if tower is destroyed */
	UFUNCTION(BlueprintCallable, Category="Tower")
	bool IsDestroyed() const { return bIsDestroyed; }

	/** Returns current HP */
	UFUNCTION(BlueprintCallable, Category="Tower")
	float GetCurrentHP() const { return CurrentHP; }

	/** Returns max HP */
	UFUNCTION(BlueprintCallable, Category="Tower")
	float GetMaxHP() const { return MaxHP; }

protected:

	/** Called when tower HP is depleted */
	void DestroyTower();

	/** Blueprint event for taking damage */
	UFUNCTION(BlueprintImplementableEvent, Category="Tower", meta=(DisplayName="On Tower Damaged"))
	void BP_OnTowerDamaged(float DamageTaken, float RemainingHP);

	/** Blueprint event for tower destruction */
	UFUNCTION(BlueprintImplementableEvent, Category="Tower", meta=(DisplayName="On Tower Destroyed"))
	void BP_OnTowerDestroyed();
};
