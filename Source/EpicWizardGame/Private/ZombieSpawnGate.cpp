// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpawnGate.h"
#include "ZombieCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

// Sets default values
AZombieSpawnGate::AZombieSpawnGate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the spawn area box component
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	RootComponent = SpawnArea;

	// Set default box extent - flat plane (200 wide x 200 tall x 10 deep)
	SpawnArea->SetBoxExtent(FVector(10.0f, 200.0f, 200.0f));

	// Disable collision - this is just a spawn volume
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Make it invisible by default
	SpawnArea->SetVisibility(false);
	SpawnArea->SetHiddenInGame(true);

	// Create the visual mesh component (plane)
	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetupAttachment(RootComponent);

	// Load the default plane mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded())
	{
		GateMesh->SetStaticMesh(PlaneMesh.Object);
		// Rotate plane to be vertical (standing up)
		GateMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
		GateMesh->SetRelativeScale3D(FVector(4.0f, 4.0f, 1.0f));
	}

	// Disable collision on the mesh
	GateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Hide in game but visible in editor
	GateMesh->SetHiddenInGame(true);
	GateMesh->bVisibleInReflectionCaptures = false;
	GateMesh->bVisibleInRayTracing = false;
	GateMesh->bVisibleInRealTimeSkyCaptures = false;

	// Create the spawn direction arrow
	SpawnDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnDirection"));
	SpawnDirection->SetupAttachment(RootComponent);
	SpawnDirection->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f)); // Point perpendicular to plane face
	SpawnDirection->ArrowSize = 2.0f;
	SpawnDirection->ArrowColor = FColor::Red;
	SpawnDirection->bIsScreenSizeScaled = true;
}

// Called when the game starts or when spawned
void AZombieSpawnGate::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AZombieSpawnGate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

AZombieCharacter* AZombieSpawnGate::SpawnZombie()
{
	// Check if we can spawn
	if (!CanSpawn())
	{
		return nullptr;
	}

	// Check if zombie class is set
	if (!ZombieClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZombieSpawnGate: No zombie class set!"));
		return nullptr;
	}

	// Get spawn location
	FVector SpawnLocation = GetRandomSpawnLocation();
	// Use the arrow's forward direction as spawn rotation
	FRotator SpawnRotation = SpawnDirection->GetComponentRotation();

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn the zombie
	AZombieCharacter* NewZombie = GetWorld()->SpawnActor<AZombieCharacter>(ZombieClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (NewZombie)
	{
		// Add to active zombies
		ActiveZombies.Add(NewZombie);

		// Bind to death delegate
		NewZombie->OnZombieDeath.AddDynamic(this, &AZombieSpawnGate::OnZombieDied);
	}

	return NewZombie;
}

bool AZombieSpawnGate::CanSpawn() const
{
	// Clean up any null references first
	const_cast<AZombieSpawnGate*>(this)->CleanupDeadZombies();

	// Check if we're under the limit
	return ActiveZombies.Num() < MaxActiveZombiesPerGate;
}

void AZombieSpawnGate::OnZombieDied()
{
	// Clean up dead zombies from the array
	CleanupDeadZombies();
}

void AZombieSpawnGate::CleanupDeadZombies()
{
	// Remove null or dead zombies from the array
	ActiveZombies.RemoveAll([](AZombieCharacter* Zombie)
	{
		return Zombie == nullptr || !IsValid(Zombie) || Zombie->IsDead();
	});
}

FVector AZombieSpawnGate::GetRandomSpawnLocation() const
{
	// Get the box extent
	FVector Extent = SpawnArea->GetScaledBoxExtent();
	FVector Center = GetActorLocation();

	// Generate random offset within the box
	float RandomX = FMath::RandRange(-Extent.X, Extent.X);
	float RandomY = FMath::RandRange(-Extent.Y, Extent.Y);
	float RandomZ = FMath::RandRange(-Extent.Z, Extent.Z);

	// Transform the offset by the actor's rotation
	FVector LocalOffset(RandomX, RandomY, RandomZ);
	FVector WorldOffset = GetActorRotation().RotateVector(LocalOffset);

	return Center + WorldOffset;
}

#if WITH_EDITOR
void AZombieSpawnGate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// This allows properties to update in the editor
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();

		// You can add custom logic here if needed when properties change
	}
}
#endif

