// Fill out your copyright notice in the Description page of Project Settings.

#include "ZombieCharacter.h"
#include "Tower.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// AI controlled
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Create floating health bar widget
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // Above head
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen); // Always face camera
	HealthBarWidget->SetDrawSize(FVector2D(200.0f, 20.0f));

	// Make zombies slower
	GetCharacterMovement()->MaxWalkSpeed = 200.0f; // Default is usually 600
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

	// Check all towers (use TowerAttackRange for towers, which can be larger)
	for (TActorIterator<ATower> It(GetWorld()); It; ++It)
	{
		ATower* Tower = *It;
		if (Tower && !Tower->IsDestroyed())
		{
			float TowerDistance = FVector::Dist(ZombieLocation, Tower->GetActorLocation());
			if (TowerDistance < NearestDistance && TowerDistance <= TowerAttackRange)
			{
				NearestDistance = TowerDistance;
				NearestTarget = Tower;
			}
		}
	}

	// Apply damage to nearest target in range
	if (NearestTarget)
	{
		// Check if it's a tower
		ATower* TargetTower = Cast<ATower>(NearestTarget);
		if (TargetTower)
		{
			UE_LOG(LogTemp, Error, TEXT("Zombie attacking TOWER %s for %f damage at distance %f"), *NearestTarget->GetName(), AttackDamage, NearestDistance);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Zombie attacking %s for %f damage at distance %f"), *NearestTarget->GetName(), AttackDamage, NearestDistance);
		}

		float DamageDealt = UGameplayStatics::ApplyDamage(NearestTarget, AttackDamage, AIController, this, nullptr);
		UE_LOG(LogTemp, Error, TEXT("ApplyDamage returned: %f"), DamageDealt);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Zombie has no target in range (AttackRange: %f, TowerAttackRange: %f)"), AttackRange, TowerAttackRange);
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

