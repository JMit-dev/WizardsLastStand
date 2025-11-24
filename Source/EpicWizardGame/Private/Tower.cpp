// Fill out your copyright notice in the Description page of Project Settings.

#include "Tower.h"
#include "Components/WidgetComponent.h"
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

