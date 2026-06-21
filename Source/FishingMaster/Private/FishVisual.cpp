// Fill out your copyright notice in the Description page of Project Settings.


#include "FishVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Ocean/OceanManager.h"

// Sets default values
AFishVisual::AFishVisual()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootComp;

	FishMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FishMesh"));
	FishMesh->SetupAttachment(RootComponent);

	// no collsion
	FishMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	SkeletalFishMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalFishMesh"));
	SkeletalFishMesh->SetupAttachment(RootComponent);
	SkeletalFishMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFishVisual::PlayDespawnEffect()
{
	Destroy();
}

// Called when the game starts or when spawned
void AFishVisual::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFishVisual::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

AFishCaughtVisual::AFishCaughtVisual()
{

}

AFishBombedVisual::AFishBombedVisual()
{
    PrimaryActorTick.bCanEverTick = true;

	PickupCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupCollision"));
	PickupCollision->SetupAttachment(RootComponent);
	
	PickupCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	PickupCollision->SetBoxExtent(FVector(100.f, 100.f, 100.f));
}

void AFishBombedVisual::BeginPlay()
{
    Super::BeginPlay();

    OceanManager = Cast<AOceanManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AOceanManager::StaticClass()));

    if (!OceanManager)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Dead Fish warning£ºNO OceanManager£¡"));
    }
    else
    {
        //FString Msg = FString::Printf(TEXT("Dead fish successfully spawned! Spawn coordinates: %s"), *GetActorLocation().ToString());
        //GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, Msg);
    }

    RandomBobbingPhase = FMath::RandRange(0.0f, PI * 2.0f);

    if (PickupCollision)
    {
        // init min defalut box size
        FVector MeshExtents = FVector(50.f, 50.f, 50.f);

        //Skeletal Mesh
        if (SkeletalFishMesh && SkeletalFishMesh->GetSkeletalMeshAsset())
        {

            MeshExtents = SkeletalFishMesh->Bounds.BoxExtent;
        }
        // Static Mesh
        else if (FishMesh && FishMesh->GetStaticMesh())
        {
            MeshExtents = FishMesh->Bounds.BoxExtent;
        }

        // Add padding
        FVector FinalExtents = MeshExtents + FVector(20.f, 20.f, 20.f);

        // min size
        FinalExtents.X = FMath::Max(FinalExtents.X, 30.f);
        FinalExtents.Y = FMath::Max(FinalExtents.Y, 30.f);
        FinalExtents.Z = FMath::Max(FinalExtents.Z, 30.f);

        PickupCollision->SetBoxExtent(FinalExtents);

        UE_LOG(LogTemp, Warning, TEXT("Dead Fish Collision Box: X=%f, Y=%f, Z=%f"),
            FinalExtents.X, FinalExtents.Y, FinalExtents.Z);
    }
}

void AFishBombedVisual::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!OceanManager) return;

    FVector CurrentLoc = GetActorLocation();
    FRotator CurrentRot = GetActorRotation();

    float TargetWaterZ = OceanManager->GetApproximateOceanHeight(CurrentLoc);

    if (bIsOnSurface)
    {
        //float RepulsionOffsetX = 0.0f;
        //float RepulsionOffsetY = 0.0f;


        //if (PushingActor != nullptr)
        //{
        //    FVector PushingLoc = PushingActor->GetActorLocation();
        //    float DistanceXY = FVector::Dist2D(CurrentLoc, PushingLoc);

        //    if (DistanceXY < RepulsionRadius && DistanceXY > 1.0f)
        //    {
        //        FVector PushDir = (CurrentLoc - PushingLoc);
        //        PushDir.Z = 0;
        //        PushDir.Normalize();

        //        float PushMultiplier = 1.0f - (DistanceXY / RepulsionRadius);
        //        RepulsionOffsetX = PushDir.X * PushMultiplier * RepulsionForce * DeltaTime;
        //        RepulsionOffsetY = PushDir.Y * PushMultiplier * RepulsionForce * DeltaTime;
        //    }
        //}

        float SmoothedZ = FMath::FInterpTo(CurrentLoc.Z, TargetWaterZ, DeltaTime, 10.0f);
        SetActorLocation(FVector(CurrentLoc.X, CurrentLoc.Y, SmoothedZ));

        // Add shake
        float Time = GetWorld()->GetTimeSeconds();
        float BobbingPitch = FMath::Sin(Time * 2.0f + RandomBobbingPhase) * 10.0f;
        float BobbingRoll = FMath::Cos(Time * 1.5f + RandomBobbingPhase) * 15.0f;

        FRotator TargetRot = FRotator(BobbingPitch, CurrentRot.Yaw, 180.0f + BobbingRoll);

        FRotator SmoothedRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 3.0f);
        SetActorRotation(SmoothedRot);
    }
    else
    {
        // emmerge
        float NewZ = FMath::FInterpConstantTo(CurrentLoc.Z, TargetWaterZ, DeltaTime, FloatUpSpeed);
        SetActorLocation(FVector(CurrentLoc.X, CurrentLoc.Y, NewZ));

        // close to surface
        if (FMath::IsNearlyEqual(NewZ, TargetWaterZ, 10.0f) || NewZ > TargetWaterZ)
        {
            bIsOnSurface = true;
        }
    }
}

//void AFishBombedVisual::SetPushedBy(AActor* InActor)
//{
//    PushingActor = InActor;
//}
