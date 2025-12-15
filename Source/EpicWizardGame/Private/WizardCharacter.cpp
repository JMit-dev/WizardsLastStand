// Fill out your copyright notice in the Description page of Project Settings.

#include "WizardCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "WizardPlayerController.h"
#include "HotbarWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Misc/PackageName.h"

AWizardCharacter::AWizardCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create first person camera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(-10.0f, 0.0f, 60.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	// Create top-down camera rig (disabled by default)
	TopDownSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("TopDownSpringArm"));
	TopDownSpringArm->SetupAttachment(GetCapsuleComponent());
	TopDownSpringArm->TargetArmLength = 900.0f;
	TopDownSpringArm->SetRelativeRotation(FRotator(TopCameraAngle, 0.0f, 0.0f));
	TopDownSpringArm->SetUsingAbsoluteRotation(true);
	TopDownSpringArm->bUsePawnControlRotation = false;
	TopDownSpringArm->bDoCollisionTest = true;
	TopDownSpringArm->bInheritPitch = false;
	TopDownSpringArm->bInheritYaw = false;
	TopDownSpringArm->bInheritRoll = false;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(TopDownSpringArm, USpringArmComponent::SocketName);
	TopDownCamera->bUsePawnControlRotation = false;
	TopDownCamera->SetActive(false);

	// Create first person mesh (arms)
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->bCastDynamicShadow = false;
	FirstPersonMesh->CastShadow = false;

	// Create staff mesh
	StaffMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaffMesh"));
	StaffMesh->SetupAttachment(FirstPersonMesh, StaffSocketName);
	StaffMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Hide the default third person mesh in first person
	GetMesh()->SetOwnerNoSee(true);

	// Default to wizard cast animation sequence
	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> WizardCastAsset(TEXT("/Game/WizardsLastStand/Assets/Characters/WizardCastSpell.WizardCastSpell"));
	if (WizardCastAsset.Succeeded())
	{
		SpellCastAnimation = WizardCastAsset.Object;
	}
}

void AWizardCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Ensure HP is initialized even if Blueprint skips calling Super::BeginPlay
	MaxHP = FMath::Max(MaxHP, 1.0f);
	CurrentHP = MaxHP;
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);
}

void AWizardCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize HP
	MaxHP = FMath::Max(MaxHP, 1.0f); // prevent zero/negative max that can break UI ratios
	CurrentHP = MaxHP;
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);

	// Attach staff to socket
	if (StaffMesh && FirstPersonMesh)
	{
		StaffMesh->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, StaffSocketName);
	}

	// Lock to top-down when on specific levels
	TryActivateTopDownView();

	// Add input mapping context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AWizardCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateMovementAnimation();
}

void AWizardCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AWizardCharacter::MoveInput);
		}

		// Looking
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWizardCharacter::LookInput);
		}

		// Jumping
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		// Cast spell (left click)
		if (CastSpellAction)
		{
			// Fire immediately on press, and keep firing while held (cooldown-gated)
			EnhancedInputComponent->BindAction(CastSpellAction, ETriggerEvent::Started, this, &AWizardCharacter::DoCastSpell);
			EnhancedInputComponent->BindAction(CastSpellAction, ETriggerEvent::Triggered, this, &AWizardCharacter::DoCastSpell);
		}

		// Hotbar scroll (mouse wheel)
		if (HotbarScrollAction)
		{
			EnhancedInputComponent->BindAction(HotbarScrollAction, ETriggerEvent::Triggered, this, &AWizardCharacter::HotbarScrollInput);
		}

		// Hotbar slot selections (number keys)
		if (HotbarSlot1Action)
		{
			EnhancedInputComponent->BindAction(HotbarSlot1Action, ETriggerEvent::Started, this, &AWizardCharacter::SelectHotbarSlot1);
		}
		if (HotbarSlot2Action)
		{
			EnhancedInputComponent->BindAction(HotbarSlot2Action, ETriggerEvent::Started, this, &AWizardCharacter::SelectHotbarSlot2);
		}
		if (HotbarSlot3Action)
		{
			EnhancedInputComponent->BindAction(HotbarSlot3Action, ETriggerEvent::Started, this, &AWizardCharacter::SelectHotbarSlot3);
		}
		if (HotbarSlot4Action)
		{
			EnhancedInputComponent->BindAction(HotbarSlot4Action, ETriggerEvent::Started, this, &AWizardCharacter::SelectHotbarSlot4);
		}
		if (HotbarSlot5Action)
		{
			EnhancedInputComponent->BindAction(HotbarSlot5Action, ETriggerEvent::Started, this, &AWizardCharacter::SelectHotbarSlot5);
		}
	}
}

float AWizardCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHP -= Damage;
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);

	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	return Damage;
}

void AWizardCharacter::MoveInput(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		if (bIsTopDownViewActive && TopDownCamera)
		{
			FVector ForwardDirection = TopDownCamera->GetForwardVector();
			FVector RightDirection = TopDownCamera->GetRightVector();

			ForwardDirection.Z = 0.0f;
			RightDirection.Z = 0.0f;

			if (!ForwardDirection.IsNearlyZero())
			{
				ForwardDirection.Normalize();
			}

			if (!RightDirection.IsNearlyZero())
			{
				RightDirection.Normalize();
			}

			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
			return;
		}

		// Get forward and right vectors (first-person)
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Add movement input
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AWizardCharacter::LookInput(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (bIsTopDownViewActive)
	{
		// Top-down camera is locked; ignore look input
		return;
	}

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AWizardCharacter::DoCastSpell()
{
	// Don't cast if already casting (prevents animation cancelling)
	if (bIsCasting)
	{
		return;
	}

	// Don't cast if dead
	if (CurrentHP <= 0.0f)
	{
		return;
	}

	// Use hotbar spell instead of old system
	AWizardPlayerController* PC = Cast<AWizardPlayerController>(GetController());
	if (PC)
	{
		UHotbarWidget* HotbarWidget = PC->GetHotbarWidget();
		if (HotbarWidget)
		{
			HotbarWidget->UseCurrentSlot();
		}
	}

	// Fallback: If no montage or sequence, just trigger the spell effect
	if (!SpellCastMontage && !SpellCastAnimation)
	{
		BP_OnSpellCast();
		return;
	}

	USkeletalMeshComponent* MeshComp = nullptr;
	if (bIsTopDownViewActive && GetMesh())
	{
		MeshComp = GetMesh(); // visible in top-down
	}
	else
	{
		MeshComp = FirstPersonMesh ? FirstPersonMesh : GetMesh();
	}
	UAnimInstance* AnimInstance = MeshComp ? MeshComp->GetAnimInstance() : nullptr;

	// Use montage if provided, otherwise fall back to single-node animation sequence
	if (SpellCastMontage && AnimInstance)
	{
		bIsCasting = true;

		float MontageLength = AnimInstance->Montage_Play(SpellCastMontage);

		if (MontageLength > 0.0f)
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AWizardCharacter::OnCastMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, SpellCastMontage);
		}
		else
		{
			bIsCasting = false;
		}
	}
	else if (SpellCastAnimation && MeshComp)
	{
		bIsCasting = true;

		SavedCastAnimMode = MeshComp->GetAnimationMode();
		SavedCastAnimClass = MeshComp->GetAnimClass();
		CastSingleNodeMesh = MeshComp;
		bUsingCastSingleNode = true;

		MeshComp->PlayAnimation(SpellCastAnimation, false);

		if (UAnimSingleNodeInstance* SingleNode = MeshComp->GetSingleNodeInstance())
		{
			SingleNode->SetPlayRate(SpellCastAnimPlayRate);
			const float Duration = SingleNode->GetLength() / FMath::Max(SpellCastAnimPlayRate, KINDA_SMALL_NUMBER);
			GetWorld()->GetTimerManager().ClearTimer(CastAnimationTimer);
			GetWorld()->GetTimerManager().SetTimer(CastAnimationTimer, this, &AWizardCharacter::OnCastAnimationFinished, Duration, false);
		}
		else
		{
			OnCastAnimationFinished();
		}
	}

	// Trigger spell effect
	BP_OnSpellCast();
}

void AWizardCharacter::OnCastMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsCasting = false;
}

void AWizardCharacter::OnCastAnimationFinished()
{
	GetWorld()->GetTimerManager().ClearTimer(CastAnimationTimer);

	if (bUsingCastSingleNode)
	{
		if (USkeletalMeshComponent* CastMesh = CastSingleNodeMesh.Get())
		{
			CastMesh->Stop();
			if (SavedCastAnimMode == EAnimationMode::AnimationBlueprint && SavedCastAnimClass)
			{
				CastMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
				CastMesh->SetAnimInstanceClass(SavedCastAnimClass);
			}
			else
			{
				CastMesh->SetAnimationMode(SavedCastAnimMode);
			}
		}
		bUsingCastSingleNode = false;
		CastSingleNodeMesh = nullptr;
		SavedCastAnimClass = nullptr;
	}

	bIsCasting = false;
}

bool AWizardCharacter::GetAimData(FVector& OutOrigin, FVector& OutDirection) const
{
	// Top-down: aim where the cursor points on the world, fire from the character
	if (bIsTopDownViewActive)
	{
		FVector Origin = GetActorLocation();
		Origin.Z += 50.0f; // raise slightly so projectiles don't spawn inside the ground

		FVector Direction = GetActorForwardVector();

		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			FHitResult CursorHit;
			if (PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit))
			{
				FVector Target = CursorHit.ImpactPoint;
				Target.Z = Origin.Z;
				Direction = (Target - Origin).GetSafeNormal();
			}
		}

		if (Direction.IsNearlyZero())
		{
			Direction = GetActorForwardVector();
		}

		OutOrigin = Origin;
		OutDirection = Direction;
		return true;
	}

	// First-person: use the first person camera
	if (FirstPersonCamera)
	{
		OutOrigin = FirstPersonCamera->GetComponentLocation();
		OutDirection = FirstPersonCamera->GetForwardVector();
		return true;
	}

	return false;
}

void AWizardCharacter::TryActivateTopDownView()
{
	bIsTopDownViewActive = ShouldUseTopDownCamera();

	if (!bIsTopDownViewActive)
	{
		// ensure first-person view is active by default
		if (FirstPersonCamera)
		{
			FirstPersonCamera->SetActive(true);
		}
		if (TopDownCamera)
		{
			TopDownCamera->SetActive(false);
		}
		return;
	}

	// Enable top-down camera and disable first-person
	if (TopDownCamera)
	{
		TopDownCamera->SetActive(true);
	}
	if (FirstPersonCamera)
	{
		FirstPersonCamera->SetActive(false);
	}

	// Show the full character model and hide first-person arms
	GetMesh()->SetOwnerNoSee(false);
	if (FirstPersonMesh)
	{
		FirstPersonMesh->SetHiddenInGame(true);
	}

	// Orient movement to direction instead of controller rotation
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = true;
		Movement->bUseControllerDesiredRotation = false;
	}

	// Disable controller rotation on the pawn
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Make sure we are the active view target
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		PC->SetViewTarget(this);
	}
}

bool AWizardCharacter::ShouldUseTopDownCamera() const
{
	if (const UWorld* World = GetWorld())
	{
		const FString MapName = FPackageName::GetShortName(World->GetMapName());
		return MapName.Contains(TEXT("Courtyard"), ESearchCase::IgnoreCase)
			|| MapName.Contains(TEXT("Rampart"), ESearchCase::IgnoreCase);
	}

	return false;
}

void AWizardCharacter::Die()
{
	bIsCasting = false;
	DisableInput(nullptr);
	GetCharacterMovement()->StopMovementImmediately();
	GetWorld()->GetTimerManager().ClearTimer(CastAnimationTimer);
	OnCastAnimationFinished();
	BP_OnDeath();

	// Return to title screen after a short delay
	FTimerHandle ReturnToTitleTimer;
	GetWorld()->GetTimerManager().SetTimer(ReturnToTitleTimer, [this]()
	{
		UGameplayStatics::OpenLevel(this, FName("TitleScreen"));
	}, 2.0f, false);
}

void AWizardCharacter::UpdateMovementAnimation()
{
	// Drive locomotion on the main mesh; fall back to first-person mesh only if needed
	USkeletalMeshComponent* MeshComp = GetMesh() ? GetMesh() : FirstPersonMesh;

	if (!MeshComp)
	{
		return;
	}

	if (!WalkAnimation)
	{
		return;
	}

	// Stop walking when casting or dead
	if (bIsCasting || CurrentHP <= 0.0f)
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

	// Determine if we should be playing the walk loop
	bool bShouldPlayWalk = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		const FVector HorizontalVelocity = FVector(MoveComp->Velocity.X, MoveComp->Velocity.Y, 0.0f);
		bShouldPlayWalk = HorizontalVelocity.SizeSquared() >= FMath::Square(WalkVelocityThreshold);
	}

	if (bShouldPlayWalk)
	{
		// Use single-node play (no slots required)
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
void AWizardCharacter::HotbarScrollInput(const FInputActionValue& Value)
{
	float ScrollValue = Value.Get<float>();

	if (AWizardPlayerController* PC = Cast<AWizardPlayerController>(GetController()))
	{
		if (UHotbarWidget* HotbarWidget = PC->GetHotbarWidget())
		{
			if (ScrollValue > 0.0f)
			{
				HotbarWidget->SelectPreviousSlot();
			}
			else if (ScrollValue < 0.0f)
			{
				HotbarWidget->SelectNextSlot();
			}
		}
	}
}

void AWizardCharacter::SelectHotbarSlot1()
{
	if (AWizardPlayerController* PC = Cast<AWizardPlayerController>(GetController()))
	{
		if (UHotbarWidget* HotbarWidget = PC->GetHotbarWidget())
		{
			HotbarWidget->SelectSlotByNumber(1);
		}
	}
}

void AWizardCharacter::SelectHotbarSlot2()
{
	if (AWizardPlayerController* PC = Cast<AWizardPlayerController>(GetController()))
	{
		if (UHotbarWidget* HotbarWidget = PC->GetHotbarWidget())
		{
			HotbarWidget->SelectSlotByNumber(2);
		}
	}
}

void AWizardCharacter::SelectHotbarSlot3()
{
	if (AWizardPlayerController* PC = Cast<AWizardPlayerController>(GetController()))
	{
		if (UHotbarWidget* HotbarWidget = PC->GetHotbarWidget())
		{
			HotbarWidget->SelectSlotByNumber(3);
		}
	}
}

void AWizardCharacter::SelectHotbarSlot4()
{
	if (AWizardPlayerController* PC = Cast<AWizardPlayerController>(GetController()))
	{
		if (UHotbarWidget* HotbarWidget = PC->GetHotbarWidget())
		{
			HotbarWidget->SelectSlotByNumber(4);
		}
	}
}

void AWizardCharacter::SelectHotbarSlot5()
{
	if (AWizardPlayerController* PC = Cast<AWizardPlayerController>(GetController()))
	{
		if (UHotbarWidget* HotbarWidget = PC->GetHotbarWidget())
		{
			HotbarWidget->SelectSlotByNumber(5);
		}
	}
}

