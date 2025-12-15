// Fill out your copyright notice in the Description page of Project Settings.

#include "Tower.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// Sets default values
ATower::ATower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create and attach the mesh
	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	RootComponent = TowerMesh;

	// Create collision box for zombie attacks - large enough to cover the tower
	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	AttackCollision->SetupAttachment(RootComponent);
	AttackCollision->SetBoxExtent(FVector(200.0f, 200.0f, 400.0f)); // Default size, can be adjusted in editor
	AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AttackCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Create floating health bar widget
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 300.0f)); // Above tower
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen); // Always face camera
	HealthBarWidget->SetDrawSize(FVector2D(300.0f, 30.0f));
}

// Called when the game starts or when spawned
void ATower::BeginPlay()
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

// Called every frame
void ATower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float ATower::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDestroyed)
	{
		return 0.0f;
	}

	CurrentHP -= Damage;

	UE_LOG(LogTemp, Warning, TEXT("Tower took %f damage! Current HP: %f / %f"), Damage, CurrentHP, MaxHP);

	// Call blueprint event for damage feedback
	BP_OnTowerDamaged(Damage, CurrentHP);

	if (CurrentHP <= 0.0f)
	{
		DestroyTower();
	}

	return Damage;
}

void ATower::DestroyTower()
{
	bIsDestroyed = true;
	CurrentHP = 0.0f;

	// Broadcast destruction
	OnTowerDestroyed.Broadcast();

	// Call blueprint event
	BP_OnTowerDestroyed();

	// Return to title screen after a short delay
	FTimerHandle ReturnToTitleTimer;
	GetWorld()->GetTimerManager().SetTimer(ReturnToTitleTimer, [this]()
	{
		UGameplayStatics::OpenLevel(this, FName("TitleScreen"));
	}, 2.0f, false);
}

