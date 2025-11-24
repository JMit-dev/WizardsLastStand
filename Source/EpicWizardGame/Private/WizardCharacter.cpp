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
#include "GameFramework/CharacterMovementComponent.h"
#include "WizardPlayerController.h"
#include "HotbarWidget.h"

AWizardCharacter::AWizardCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create first person camera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(-10.0f, 0.0f, 60.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

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
}

void AWizardCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize HP
	CurrentHP = MaxHP;

	// Attach staff to socket
	if (StaffMesh && FirstPersonMesh)
	{
		StaffMesh->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, StaffSocketName);
	}

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
			EnhancedInputComponent->BindAction(CastSpellAction, ETriggerEvent::Started, this, &AWizardCharacter::DoCastSpell);
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
		// Get forward and right vectors
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
			return;
		}
	}

	// Fallback: If no montage, just trigger the spell effect
	if (!SpellCastMontage)
	{
		BP_OnSpellCast();
		return;
	}

	// Get anim instance
	UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance();
	if (!AnimInstance)
	{
		BP_OnSpellCast();
		return;
	}

	// Set casting flag
	bIsCasting = true;

	// Play the montage
	float MontageLength = AnimInstance->Montage_Play(SpellCastMontage);

	if (MontageLength > 0.0f)
	{
		// Bind to montage end
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AWizardCharacter::OnCastMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, SpellCastMontage);

		// Trigger spell effect
		BP_OnSpellCast();
	}
	else
	{
		bIsCasting = false;
	}
}

void AWizardCharacter::OnCastMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsCasting = false;
}

void AWizardCharacter::Die()
{
	bIsCasting = false;
	DisableInput(nullptr);
	GetCharacterMovement()->StopMovementImmediately();
	BP_OnDeath();
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

