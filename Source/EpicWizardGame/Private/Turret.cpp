// Fill out your copyright notice in the Description page of Project Settings.


#include "Turret.h"
#include "ZombieCharacter.h"
#include "SpellProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"

// Sets default values
ATurret::ATurret()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetCanBeDamaged(true);

	// Stable root for C++ + BP components
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	// Create collision box for zombie attacks (overlap only)
	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	AttackCollision->SetupAttachment(RootComponent);
	AttackCollision->SetBoxExtent(FVector(120.0f, 120.0f, 120.0f));
	AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AttackCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Create floating health bar widget (same as zombies/tower)
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidget->SetDrawSize(FVector2D(200.0f, 20.0f));

	// Default to the shared floating health bar widget blueprint
	static ConstructorHelpers::FClassFinder<UUserWidget> HealthBarWidgetBP(TEXT("/Game/WizardsLastStand/UI/WBP_FloatingHealthBar"));
	if (HealthBarWidgetBP.Succeeded())
	{
		HealthBarWidget->SetWidgetClass(HealthBarWidgetBP.Class);
	}
}

// Called when the game starts or when spawned
void ATurret::BeginPlay()
{
	Super::BeginPlay();

	// Blueprint defaults can override "Can Be Damaged", so force it at runtime
	SetCanBeDamaged(!bIsPreviewTurret);

	// Ensure the widget exists and points at this turret (WBP_FloatingHealthBar expects an OwnerActor variable)
	if (HealthBarWidget)
	{
		HealthBarWidget->InitWidget();
		UE_LOG(LogTemp, Log, TEXT("Turret HealthBarWidget class: %s"), *GetNameSafe(HealthBarWidget->GetWidgetClass()));

		if (UUserWidget* UserWidget = HealthBarWidget->GetUserWidgetObject())
		{
			if (FObjectPropertyBase* OwnerActorProp = FindFProperty<FObjectPropertyBase>(UserWidget->GetClass(), TEXT("OwnerActor")))
			{
				if (OwnerActorProp->PropertyClass && OwnerActorProp->PropertyClass->IsChildOf(AActor::StaticClass()))
				{
					OwnerActorProp->SetObjectPropertyValue_InContainer(UserWidget, this);
					UE_LOG(LogTemp, Log, TEXT("Turret health bar OwnerActor set to %s"), *GetName());
				}
			}
		}
	}

	// Check if we're in a menu screen - if so, hide health bar
	FString CurrentLevelName = GetWorld()->GetMapName();
	CurrentLevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	if (CurrentLevelName.Contains(TEXT("TitleScreen")) || CurrentLevelName.Contains(TEXT("DeathScreen")))
	{
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(false);
		}
	}

	// Initialize HP
	CurrentHP = MaxHP;

	FireTimer = FireRate;
}

// Called every frame
void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDestroyed || bIsPreviewTurret)
	{
		return;
	}

	// Find nearest zombie
	AZombieCharacter* NearestZombie = FindNearestZombie();

	// If we have a valid target
	if (NearestZombie && !NearestZombie->IsDead())
	{
		// Update fire timer
		FireTimer -= DeltaTime;

		// Shoot if timer is ready
		if (FireTimer <= 0.0f)
		{
			ShootAtTarget(NearestZombie);
			FireTimer = FireRate;
		}
	}
	else
	{
		// Reset timer when no target
		FireTimer = FireRate;
	}
}

// Called to bind functionality to input
void ATurret::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ATurret::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (bIsDestroyed || bIsPreviewTurret)
	{
		return 0.0f;
	}

	const float AppliedDamage = FMath::Max(0.0f, Damage);
	if (AppliedDamage <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHP -= AppliedDamage;

	UE_LOG(LogTemp, Warning, TEXT("Turret took %f damage! Current HP: %f / %f"), AppliedDamage, CurrentHP, MaxHP);

	BP_OnTurretDamaged(AppliedDamage, CurrentHP);

	if (CurrentHP <= 0.0f)
	{
		DestroyTurret();
	}

	return AppliedDamage;
}

void ATurret::SetIsPreviewTurret(bool bIsPreview)
{
	bIsPreviewTurret = bIsPreview;

	SetCanBeDamaged(!bIsPreviewTurret);

	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(!bIsPreviewTurret);
	}
}

AZombieCharacter* ATurret::FindNearestZombie()
{
	TArray<AActor*> FoundZombies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombieCharacter::StaticClass(), FoundZombies);

	AZombieCharacter* NearestZombie = nullptr;
	float NearestDistance = DetectionRange;

	for (AActor* Actor : FoundZombies)
	{
		AZombieCharacter* Zombie = Cast<AZombieCharacter>(Actor);
		if (Zombie && !Zombie->IsDead())
		{
			float Distance = FVector::Dist(GetActorLocation(), Zombie->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestZombie = Zombie;
			}
		}
	}

	return NearestZombie;
}

void ATurret::ShootAtTarget(AZombieCharacter* Target)
{
	if (!ProjectileClass || !Target)
	{
		return;
	}

	// Calculate direction to target
	FVector TurretLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector Direction = (TargetLocation - TurretLocation).GetSafeNormal();

	// Apply aim pitch bias to adjust travel arc
	FRotator AimRotation = Direction.Rotation();
	AimRotation.Pitch += ProjectileAimPitchOffset;
	Direction = AimRotation.Vector();

	// Spawn projectile at turret location with offset upward
	FVector SpawnLocation = TurretLocation + FVector(0, 0, ProjectileVerticalOffset);
	FRotator SpawnRotation = Direction.Rotation();
	SpawnRotation.Pitch += ProjectileVisualPitchOffset; // visual-only tweak

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (Projectile)
	{
		// Initialize the projectile with direction and damage
		Projectile->InitializeProjectile(Direction, ProjectileDamage);
	}
}

void ATurret::DestroyTurret()
{
	if (bIsDestroyed)
	{
		return;
	}

	bIsDestroyed = true;
	CurrentHP = 0.0f;

	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}

	BP_OnTurretDestroyed();

	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	Destroy();
}
