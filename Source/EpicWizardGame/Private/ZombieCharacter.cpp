// Fill out your copyright notice in the Description page of Project Settings.

#include "ZombieCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

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
	// Find player in range
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	// Check distance
	float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (Distance <= AttackRange)
	{
		// Apply damage
		UGameplayStatics::ApplyDamage(PlayerPawn, AttackDamage, GetController(), this, nullptr);
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

