// Fill out your copyright notice in the Description page of Project Settings.

#include "ZombieCharacter.h"
#include "Tower.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

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

	// Default to the zombie-specific walk animation
	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> ZombieWalkAsset(TEXT("/Game/WizardsLastStand/Assets/Characters/ZombieWalk.ZombieWalk"));
	if (ZombieWalkAsset.Succeeded())
	{
		WalkAnimation = ZombieWalkAsset.Object;
	}

	// Default to the zombie-specific attack animation
	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> ZombieAttackAsset(TEXT("/Game/WizardsLastStand/Assets/Characters/ZombieAttack.ZombieAttack"));
	if (ZombieAttackAsset.Succeeded())
	{
		AttackAnimation = ZombieAttackAsset.Object;
	}
}

void AZombieCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Check if we're in the title screen - if so, hide health bar
	FString CurrentLevelName = GetWorld()->GetMapName();
	CurrentLevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	if (CurrentLevelName.Contains(TEXT("TitleScreen")))
	{
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(false);
		}
	}

	// Initialize HP
	CurrentHP = MaxHP;
}

void AZombieCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Clear timers
	GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
	GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);
}

void AZombieCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateMovementAnimation();
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
	if (!AttackMontage && !AttackAnimation)
	{
		ApplyAttackDamage();
		BP_OnAttack();
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	UAnimInstance* AnimInstance = MeshComp ? MeshComp->GetAnimInstance() : nullptr;

	// Use montage if provided, otherwise fall back to single-node animation
	if (AttackMontage && AnimInstance)
	{
		bIsAttacking = true;

		// Play attack montage
		float MontageLength = AnimInstance->Montage_Play(AttackMontage);

		if (MontageLength > 0.0f)
		{
			// Bind to montage end
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AZombieCharacter::OnAttackMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
		}
		else
		{
			bIsAttacking = false;
		}
	}
	else if (AttackAnimation && MeshComp)
	{
		bIsAttacking = true;

		// Stop walk loop while attacking
		if (bUsingSingleNodeWalk)
		{
			if (USkeletalMeshComponent* WalkMesh = WalkSingleNodeMesh.Get())
			{
				WalkMesh->Stop();
				if (SavedWalkAnimMode == EAnimationMode::AnimationBlueprint && SavedWalkAnimClass)
				{
					WalkMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
					WalkMesh->SetAnimInstanceClass(SavedWalkAnimClass);
				}
				else
				{
					WalkMesh->SetAnimationMode(SavedWalkAnimMode);
				}
			}
			bUsingSingleNodeWalk = false;
			WalkSingleNodeMesh = nullptr;
			SavedWalkAnimClass = nullptr;
		}

		SavedAttackAnimMode = MeshComp->GetAnimationMode();
		SavedAttackAnimClass = MeshComp->GetAnimClass();
		AttackSingleNodeMesh = MeshComp;
		bUsingAttackSingleNode = true;

		MeshComp->PlayAnimation(AttackAnimation, false);

		if (UAnimSingleNodeInstance* SingleNode = MeshComp->GetSingleNodeInstance())
		{
			SingleNode->SetPlayRate(AttackAnimPlayRate);
			const float Duration = SingleNode->GetLength() / FMath::Max(AttackAnimPlayRate, KINDA_SMALL_NUMBER);
			GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);
			GetWorld()->GetTimerManager().SetTimer(AttackAnimationTimer, this, &AZombieCharacter::OnAttackAnimationFinished, Duration, false);
		}
		else
		{
			OnAttackAnimationFinished();
		}
	}

	// Apply damage (you could also use anim notifies for timing)
	ApplyAttackDamage();
	BP_OnAttack();
}

void AZombieCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
}

void AZombieCharacter::OnAttackAnimationFinished()
{
	GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);

	if (bUsingAttackSingleNode)
	{
		if (USkeletalMeshComponent* AttackMesh = AttackSingleNodeMesh.Get())
		{
			AttackMesh->Stop();
			if (SavedAttackAnimMode == EAnimationMode::AnimationBlueprint && SavedAttackAnimClass)
			{
				AttackMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
				AttackMesh->SetAnimInstanceClass(SavedAttackAnimClass);
			}
			else
			{
				AttackMesh->SetAnimationMode(SavedAttackAnimMode);
			}
		}
		bUsingAttackSingleNode = false;
		AttackSingleNodeMesh = nullptr;
		SavedAttackAnimClass = nullptr;
	}

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

	// Check all towers using collision overlap instead of distance
	// This allows zombies to damage vertically stretched towers
	for (TActorIterator<ATower> It(GetWorld()); It; ++It)
	{
		ATower* Tower = *It;
		if (Tower && !Tower->IsDestroyed())
		{
			// Check if zombie's capsule is overlapping with the tower's attack collision
			TArray<AActor*> OverlappingActors;
			GetOverlappingActors(OverlappingActors, ATower::StaticClass());

			bool bIsOverlapping = OverlappingActors.Contains(Tower);

			if (bIsOverlapping)
			{
				// Prioritize tower if we're overlapping with it
				float TowerDistance = FVector::Dist(ZombieLocation, Tower->GetActorLocation());
				if (TowerDistance < NearestDistance)
				{
					NearestDistance = TowerDistance;
					NearestTarget = Tower;
				}
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
		UE_LOG(LogTemp, Log, TEXT("Zombie has no target in range (AttackRange: %f)"), AttackRange);
	}
}

void AZombieCharacter::Die()
{
	bIsDead = true;
	bIsAttacking = false;

	GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimer);
	OnAttackAnimationFinished();

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

void AZombieCharacter::UpdateMovementAnimation()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp || !WalkAnimation)
	{
		return;
	}

	// Stop walking when attacking or dead
	if (bIsAttacking || bIsDead)
	{
		if (bUsingSingleNodeWalk)
		{
			if (USkeletalMeshComponent* WalkMesh = WalkSingleNodeMesh.Get())
			{
				WalkMesh->Stop();
				if (SavedWalkAnimMode == EAnimationMode::AnimationBlueprint && SavedWalkAnimClass)
				{
					WalkMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
					WalkMesh->SetAnimInstanceClass(SavedWalkAnimClass);
				}
				else
				{
					WalkMesh->SetAnimationMode(SavedWalkAnimMode);
				}
			}
			bUsingSingleNodeWalk = false;
			WalkSingleNodeMesh = nullptr;
			SavedWalkAnimClass = nullptr;
		}
		return;
	}

	bool bShouldPlayWalk = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		const FVector HorizontalVelocity = FVector(MoveComp->Velocity.X, MoveComp->Velocity.Y, 0.0f);
		bShouldPlayWalk = HorizontalVelocity.SizeSquared() >= FMath::Square(WalkVelocityThreshold);
	}

	if (bShouldPlayWalk)
	{
		if (!bUsingSingleNodeWalk || WalkSingleNodeMesh.Get() != MeshComp)
		{
			SavedWalkAnimMode = MeshComp->GetAnimationMode();
			SavedWalkAnimClass = MeshComp->GetAnimClass();
			WalkSingleNodeMesh = MeshComp;
			bUsingSingleNodeWalk = true;

			MeshComp->PlayAnimation(WalkAnimation, true);
		}

		if (UAnimSingleNodeInstance* SingleNode = MeshComp->GetSingleNodeInstance())
		{
			SingleNode->SetPlayRate(WalkAnimPlayRate);
		}
	}
	else if (bUsingSingleNodeWalk)
	{
		if (USkeletalMeshComponent* WalkMesh = WalkSingleNodeMesh.Get())
		{
			WalkMesh->Stop();
			if (SavedWalkAnimMode == EAnimationMode::AnimationBlueprint && SavedWalkAnimClass)
			{
				WalkMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
				WalkMesh->SetAnimInstanceClass(SavedWalkAnimClass);
			}
			else
			{
				WalkMesh->SetAnimationMode(SavedWalkAnimMode);
			}
		}
		bUsingSingleNodeWalk = false;
		WalkSingleNodeMesh = nullptr;
		SavedWalkAnimClass = nullptr;
	}
}
