// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APortal::APortal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create trigger box
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 200.0f)); // Wide enough for player
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAll"));
	TriggerBox->SetGenerateOverlapEvents(true);

	// Create direction arrow
	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
	DirectionArrow->SetupAttachment(RootComponent);
	DirectionArrow->SetArrowColor(FLinearColor::Blue);
	DirectionArrow->ArrowSize = 2.0f;
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap event
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnTriggerBeginOverlap);
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update cooldowns
	TArray<AActor*> ActorsToRemove;
	for (auto& Pair : CooldownMap)
	{
		Pair.Value -= DeltaTime;
		if (Pair.Value <= 0.0f)
		{
			ActorsToRemove.Add(Pair.Key);
		}
	}

	// Remove expired cooldowns
	for (AActor* Actor : ActorsToRemove)
	{
		CooldownMap.Remove(Actor);
	}
}

void APortal::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !LinkedPortal)
	{
		return;
	}

	// Don't teleport if on cooldown
	if (IsOnCooldown(OtherActor))
	{
		return;
	}

	// Check if actor is moving toward the portal (entering from arrow direction)
	FVector ToActor = OtherActor->GetActorLocation() - GetActorLocation();
	FVector PortalForward = GetActorForwardVector();
	float DotProduct = FVector::DotProduct(ToActor.GetSafeNormal(), PortalForward);

	// Only teleport if entering from the front (where arrow points away from)
	if (DotProduct > 0.0f)
	{
		TeleportActor(OtherActor);
	}
}

void APortal::TeleportActor(AActor* ActorToTeleport)
{
	if (!ActorToTeleport || !LinkedPortal)
	{
		return;
	}

	// Get current actor rotation
	FRotator CurrentRotation = ActorToTeleport->GetActorRotation();

	// Get entry and exit portal rotations
	FRotator EntryPortalRotation = GetActorRotation();
	FRotator ExitPortalRotation = LinkedPortal->GetActorRotation();

	// Calculate relative rotation (how much the actor is rotated relative to entry portal)
	float RelativeYaw = CurrentRotation.Yaw - EntryPortalRotation.Yaw;

	// Apply relative rotation to exit portal, then add 180 to face away
	FRotator CharacterRotation = ExitPortalRotation;
	CharacterRotation.Yaw += RelativeYaw + 180.0f;

	// Get exit location
	FVector ExitLocation = LinkedPortal->GetActorLocation();
	FVector ExitForward = LinkedPortal->GetActorForwardVector();
	ExitLocation += ExitForward * 150.0f; // Exit 150 units in front of the portal

	// Teleport the actor
	ActorToTeleport->SetActorLocation(ExitLocation);
	ActorToTeleport->SetActorRotation(CharacterRotation);

	// For characters, also rotate the controller (important for first-person)
	if (ACharacter* Character = Cast<ACharacter>(ActorToTeleport))
	{
		if (AController* Controller = Character->GetController())
		{
			Controller->SetControlRotation(CharacterRotation);
		}

		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			// Rotate velocity to match character's new direction
			FVector NewVelocity = CharacterRotation.RotateVector(MovementComp->Velocity);
			MovementComp->Velocity = NewVelocity;
		}
	}

	// Add both portals to cooldown to prevent instant re-teleport
	AddToCooldown(ActorToTeleport);
	LinkedPortal->AddToCooldown(ActorToTeleport);

	UE_LOG(LogTemp, Log, TEXT("Portal: Teleported %s"), *ActorToTeleport->GetName());
}

bool APortal::IsOnCooldown(AActor* Actor) const
{
	return CooldownMap.Contains(Actor);
}

void APortal::AddToCooldown(AActor* Actor)
{
	CooldownMap.Add(Actor, TeleportCooldown);
}

