// Fill out your copyright notice in the Description page of Project Settings.


#include "FishSwarmActor.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraComponent.h"
#include "FishData.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Ocean/OceanManager.h"
#include "FishingBoat.h"

// Sets default values
AFishSwarmActor::AFishSwarmActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    SwarmZone = CreateDefaultSubobject<UCapsuleComponent>(TEXT("SwarmZone"));
    RootComponent = SwarmZone;

    SwarmZone->SetCapsuleSize(800.0f, 2000.0f);
    SwarmZone->SetCollisionProfileName(TEXT("Trigger"));

    // Debug
    //SwarmZone->SetHiddenInGame(false);
    //SwarmZone->SetVisibility(true);

    SwarmZone->ShapeColor = FColor::Yellow;

    SwarmVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SwarmVFX"));
    SwarmVFX->SetupAttachment(RootComponent);

    ValueIndicatorComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ValueIndicator"));
    ValueIndicatorComp->SetupAttachment(RootComponent);
    ValueIndicatorComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AFishSwarmActor::BeginPlay()
{
	Super::BeginPlay();

    SwarmZone->OnComponentBeginOverlap.AddDynamic(this, &AFishSwarmActor::OnSwarmOverlapBegin);
    SwarmZone->OnComponentEndOverlap.AddDynamic(this, &AFishSwarmActor::OnSwarmOverlapEnd);

    // DetermineSwarmType();

    OceanManager = Cast<AOceanManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AOceanManager::StaticClass()));

    float WaterZ = OceanManager->GetApproximateOceanHeight(GetActorLocation());

    if (SwarmVFX)
    {
        SwarmVFX->SetFloatParameter(FName("User.WaterSurfaceZ"), WaterZ - 100.0f);
    }
}

FDataTableRowHandle AFishSwarmActor::DrawFishFromPool()
{
    FDataTableRowHandle EmptyHandle;

    if (!CurrentConfig || CurrentConfig->FishPool.Num() == 0 || CurrentCapacity <= 0)
    {
        return EmptyHandle;
    }


    //Roulette Wheel Selection
    float TotalWeight = 0.0f;
    for (const FFishSpawnWeight& Item : CurrentConfig->FishPool)
    {
        TotalWeight += Item.Weight;
    }

    float RandomPoint = FMath::FRandRange(0.0f, TotalWeight);

    for (const FFishSpawnWeight& Item : CurrentConfig->FishPool)
    {
        RandomPoint -= Item.Weight;

        if (RandomPoint <= 0.0f)
        {
            // Get Fish
            CurrentCapacity--;

            // If no fish, destory self
            if (CurrentCapacity <= 0)
            {
                SwarmVFX->Deactivate();
                // Destory in 10s
                SetLifeSpan(10.0f);
            }

            return Item.FishRow;
        }
    }

    return CurrentConfig->FishPool.Last().FishRow;
}

void AFishSwarmActor::OnSwarmOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (AFishingBoat* Boat = Cast<AFishingBoat>(OtherActor))
    {
        // Hide the indicator when the player's boat enters the swarm area
        if (bIsIndicatorActive && ValueIndicatorComp)
        {
            bIsIndicatorActive = false;
            ValueIndicatorComp->SetVisibility(false);
        }

        // tell boat get into fish area
        if (OtherComp == Boat->GetRootComponent())
        {
            Boat->EnterSwarm(this);
        }
    }
}

void AFishSwarmActor::OnSwarmOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (AFishingBoat* Boat = Cast<AFishingBoat>(OtherActor))
    {
        // tell boat leave area
        if (OtherComp == Boat->GetRootComponent())
        {
            Boat->LeaveSwarm(this);
        }

        if (!bIsIndicatorActive && ValueIndicatorComp)
        {
            bIsIndicatorActive = true;
            ValueIndicatorComp->SetVisibility(true);
        }
    }
}

// void AFishSwarmActor::DetermineSwarmType()
// {
//     if (PossibleConfigs.Num() == 0)
//     {
//         UE_LOG(LogTemp, Error, TEXT("No PossibleConfigs ��"));
//         return;
//     }

//     int32 RandomIndex = FMath::RandRange(0, PossibleConfigs.Num() - 1);
//     CurrentConfig = PossibleConfigs[RandomIndex];

//     if (CurrentConfig)
//     {
//         CurrentCapacity = CurrentConfig->MaxCapacity;

//         if (SwarmZone)
//         {
//             SwarmZone->SetCapsuleSize(CurrentConfig->SwarmRadius, 2000.0f);
//         }

//         if (CurrentConfig->UnderwaterSwarmVFX)
//         {
//             SwarmVFX->SetAsset(CurrentConfig->UnderwaterSwarmVFX);

//             SwarmVFX->SetRelativeLocation(FVector(0.0f, 0.0f, -400.0f));

//             SwarmVFX->Activate();
//         }

//         // Value Indicator
//         if (ValueIndicatorComp && CurrentConfig->ValueIndicatorMesh)
//         {
//             ValueIndicatorComp->SetStaticMesh(CurrentConfig->ValueIndicatorMesh);
//             // Place it reasonably above the swarm center
//             IndicatorBaseZ = 500.0f;
//             ValueIndicatorComp->SetRelativeLocation(FVector(0.0f, 0.0f, IndicatorBaseZ));
//         }

//         UE_LOG(LogTemp, Warning, TEXT("Generate Fish Swarm complete: %d"), CurrentCapacity);
//     }
// }


void AFishSwarmActor::InitializeSwarm(UFishSwarmConfig* AssignedConfig)
{
    if (!AssignedConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeSwarm: AssignedConfig is null"));
        return;
    }

    CurrentConfig = AssignedConfig;

    CurrentCapacity = CurrentConfig->MaxCapacity;

    if (SwarmZone)
    {
        SwarmZone->SetCapsuleSize(CurrentConfig->SwarmRadius, 2000.0f);
    }

    //VFX
    if (CurrentConfig->UnderwaterSwarmVFX)
    {
        SwarmVFX->SetAsset(CurrentConfig->UnderwaterSwarmVFX);
        SwarmVFX->SetRelativeLocation(FVector(0.0f, 0.0f, -400.0f));
        SwarmVFX->Activate();
    }

    // Value Indicator
    if (ValueIndicatorComp && CurrentConfig->ValueIndicatorMesh)
    {
        ValueIndicatorComp->SetStaticMesh(CurrentConfig->ValueIndicatorMesh);
        IndicatorBaseZ = 500.0f; 
        ValueIndicatorComp->SetRelativeLocation(FVector(0.0f, 0.0f, IndicatorBaseZ));
    }

    UE_LOG(LogTemp, Warning, TEXT("Generate Fish Swarm complete: %d"), CurrentCapacity);
}

void AFishSwarmActor::OnBombed(FVector BombLocation)
{
    if (GetLifeSpan() > 0.0f)
    {
        return;
    }

    if (!CurrentConfig || CurrentCapacity <= 0)
    {
        Destroy();
        return;
    }

    int32 MinYield = FMath::RoundToInt(CurrentConfig->BombYieldRange.X);
    int32 MaxYield = FMath::RoundToInt(CurrentConfig->BombYieldRange.Y);
    int32 DeadFishCount = FMath::Min(CurrentCapacity, FMath::RandRange(MinYield, MaxYield));

    FString SwarmLog = FString::Printf(TEXT("Swarm Bombed! Trying to spawn %d fishes."), DeadFishCount);
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, SwarmLog);

    for (int32 i = 0; i < DeadFishCount; i++)
    {
        FDataTableRowHandle ChosenFishHandle = DrawFishFromPool();

        if (!ChosenFishHandle.IsNull())
        {
            FFishSpeciesData* FishData = ChosenFishHandle.GetRow<FFishSpeciesData>(TEXT("Bombed Fish Lookup"));

            if (FishData && FishData->BombedFishClass)
            {
                float RandomX = FMath::RandRange(-400.0f, 400.0f);
                float RandomY = FMath::RandRange(-400.0f, 400.0f);
                //FVector SpawnLoc = GetActorLocation() + FVector(RandomX, RandomY, 0.0f);
                FVector SpawnLoc = GetActorLocation() + FVector(RandomX, RandomY, -500.0f);

                FActorSpawnParameters SpawnParams;
                // Ignore collisions
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                // FORCE SPAWN EVEN IF ENGINE WANTS TO FAIL
                SpawnParams.bNoFail = true;
                //SpawnParams.Owner = this;
                SpawnParams.Owner = nullptr;

                // Use FTransform which is much safer in UE5
                FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLoc);

                AFishBombedVisual* DeadFish = GetWorld()->SpawnActor<AFishBombedVisual>(
                    FishData->BombedFishClass,
                    SpawnTransform,
                    SpawnParams
                );

                if (DeadFish)
                {
                    DeadFish->FishDataHandle = ChosenFishHandle;
                    //DeadFish->StartFloatingUp();
                }
                else
                {
                    // [DEBUG] If it STILL fails, print exactly what class and where
                    FString ErrMsg = FString::Printf(TEXT("CRITICAL ERROR: SpawnActor failed for %s at %s!"),
                        *FishData->BombedFishClass->GetName(),
                        *SpawnLoc.ToString());
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, ErrMsg);
                    UE_LOG(LogTemp, Error, TEXT("%s"), *ErrMsg);
                }
            }
            else
            {
                // [DEBUG] Missing class in DataTable
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("ERROR: BombedFishClass is NULL in DataTable!"));
            }
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("CRITICAL: DrawFishFromPool Return NUll��Check FishSwarmConfig DataAsset��"));
        }
    }

    CurrentCapacity = 0;
    if (SwarmVFX)
    {
        SwarmVFX->SetFloatParameter(FName("User.AttractionStrength"), 2000.0f);

        // SwarmVFX->Deactivate();
    }

    //Destroy();
    SetLifeSpan(3.0f);
}

// Called every frame
void AFishSwarmActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (bIsIndicatorActive && ValueIndicatorComp && CurrentConfig && CurrentConfig->ValueIndicatorMesh)
    {
        // Add Rotation
        FRotator NewRotation = ValueIndicatorComp->GetRelativeRotation();
        NewRotation.Yaw += CurrentConfig->IndicatorRotationSpeed * DeltaTime;
        ValueIndicatorComp->SetRelativeRotation(NewRotation);

        // Calculate Bobbing offset
        float BobbingOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * CurrentConfig->IndicatorBobbingSpeed) * CurrentConfig->IndicatorBobbingHeight;
        
        FVector NewLocation = ValueIndicatorComp->GetRelativeLocation();
        NewLocation.Z = IndicatorBaseZ + BobbingOffset;
        ValueIndicatorComp->SetRelativeLocation(NewLocation);
    }
}

