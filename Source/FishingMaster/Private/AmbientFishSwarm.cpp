// Fill out your copyright notice in the Description page of Project Settings.


#include "AmbientFishSwarm.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Ocean/OceanManager.h"

// Sets default values
AAmbientFishSwarm::AAmbientFishSwarm()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootComp;

	SwarmVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SwarmVFX"));
	SwarmVFX->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AAmbientFishSwarm::BeginPlay()
{
	Super::BeginPlay();
	
	OceanManager = Cast<AOceanManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AOceanManager::StaticClass()));

	if (AmbientVFX)
	{
		SwarmVFX->SetAsset(AmbientVFX);
		SwarmVFX->Activate();
	}

	StartLocation = GetActorLocation();
	PickNewTargetLocation();
}

void AAmbientFishSwarm::PickNewTargetLocation()
{
	// Gererate new location around start location
	TargetLocation = StartLocation + UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(500.0f, RoamRadius);
	// Make sure Fish bellow water
	TargetLocation.Z = StartLocation.Z + FMath::RandRange(-500.0f, 500.0f);

	if (OceanManager)
	{
		float SurfaceZ = OceanManager->GetApproximateOceanHeight(TargetLocation);
		if (TargetLocation.Z > SurfaceZ - 100.0f)
		{
			TargetLocation.Z = SurfaceZ - 100.0f;
		}
	}
	//if (TargetLocation.Z > OceanManager.GetActorLoaction())
	//{
	//	TargetLocation.Z = OceanManager.GetActorLoaction() - 100.f;
	//}
}

// Called every frame
void AAmbientFishSwarm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector CurrentLoc = GetActorLocation();
	FVector NewLoc = FMath::VInterpConstantTo(CurrentLoc, TargetLocation, DeltaTime, RoamSpeed);
	SetActorLocation(NewLoc);

	// If get target location, choose a new location
	if (FVector::DistSquared(CurrentLoc, TargetLocation) < 100.0f)
	{
		PickNewTargetLocation();
	}
}

