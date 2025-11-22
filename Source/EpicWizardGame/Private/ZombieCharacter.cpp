// Fill out your copyright notice in the Description page of Project Settings.

#include "ZombieCharacter.h"
#include "Tower.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// AI controlled
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AZombieCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize HP
	CurrentHP = MaxHP;
}

void AZombieCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Clear timers
	GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
}

float AZombieCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead)
	{
		return 0.0f;
	}

	CurrentHP -= Damage;

	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	return Damage;
}

void AZombieCharacter::DoAttack()
{
	// Don't attack if already attacking or dead
	if (bIsAttacking || bIsDead)
	{
		return;
	}

	// If no montage, just apply damage
	if (!AttackMontage)
	{
		ApplyAttackDamage();
		BP_OnAttack();
		return;
	}

	// Get anim instance
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		ApplyAttackDamage();
		BP_OnAttack();
		return;
	}

	// Set attacking flag
	bIsAttacking = true;

	// Play attack montage
	float MontageLength = AnimInstance->Montage_Play(AttackMontage);

	if (MontageLength > 0.0f)
	{
		// Bind to montage end
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AZombieCharacter::OnAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);

		// Apply damage (you could also use anim notifies for timing)
		ApplyAttackDamage();
		BP_OnAttack();
	}
	else
	{
		bIsAttacking = false;
	}
}

void AZombieCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
}

void AZombieCharacter::ApplyAttackDamage()
{
	// Get current AI target from controller
	AController* AIController = GetController();
	if (!AIController)
	{
		return;
	}

	FVector ZombieLocation = GetActorLocation();
	AActor* NearestTarget = nullptr;
	float NearestDistance = FLT_MAX;

	// Check player
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		float PlayerDistance = FVector::Dist(ZombieLocation, PlayerPawn->GetActorLocation());
		if (PlayerDistance < NearestDistance && PlayerDistance <= AttackRange)
		{
			NearestDistance = PlayerDistance;
			NearestTarget = PlayerPawn;
		}
	}

	// Check all towers
	for (TActorIterator<ATower> It(GetWorld()); It; ++It)
	{
		ATower* Tower = *It;
		if (Tower && !Tower->IsDestroyed())
		{
			float TowerDistance = FVector::Dist(ZombieLocation, Tower->GetActorLocation());
			if (TowerDistance < NearestDistance && TowerDistance <= AttackRange)
			{
				NearestDistance = TowerDistance;
				NearestTarget = Tower;
			}
		}
	}

	// Apply damage to nearest target in range
	if (NearestTarget)
	{
		UGameplayStatics::ApplyDamage(NearestTarget, AttackDamage, AIController, this, nullptr);
	}
}

void AZombieCharacter::Die()
{
	bIsDead = true;
	bIsAttacking = false;

	// Stop movement
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Broadcast death
	OnZombieDeath.Broadcast();

	// Call blueprint event
	BP_OnDeath();

	// Schedule destruction
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &AZombieCharacter::DeferredDestruction, DeathDestroyDelay, false);
}

void AZombieCharacter::DeferredDestruction()
{
	Destroy();
}

