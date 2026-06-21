// Fill out your copyright notice in the Description page of Project Settings.


#include "FishSpawnManager.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Ocean/OceanManager.h"
#include "Config/FishManagerConfig.h"
#include "FishSwarmActor.h"

// Sets default values
AFishSpawnManager::AFishSpawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	RootComponent = SpawnArea;
	SpawnArea->SetBoxExtent(FVector(5000.f, 5000.f, 1000.f));
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    LimitationPlane = CreateDefaultSubobject<UBoxComponent>(TEXT("LimitationPlane"));
    LimitationPlane->SetupAttachment(RootComponent);
    LimitationPlane->SetBoxExtent(FVector(10000.f, 10000.f, 10.f));
    //SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);

// North Wall
    NorthWall = CreateDefaultSubobject<UBoxComponent>(TEXT("NorthWall"));
    NorthWall->SetupAttachment(SpawnArea);
    NorthWall->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    NorthWall->SetCollisionResponseToAllChannels(ECR_Ignore);
    NorthWall->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    NorthWall->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
    NorthWall->SetHiddenInGame(true);

    // South Wall
    SouthWall = CreateDefaultSubobject<UBoxComponent>(TEXT("SouthWall"));
    SouthWall->SetupAttachment(SpawnArea);
    SouthWall->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SouthWall->SetCollisionResponseToAllChannels(ECR_Ignore);
    SouthWall->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    SouthWall->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
    SouthWall->SetHiddenInGame(true);

    // East Wall
    EastWall = CreateDefaultSubobject<UBoxComponent>(TEXT("EastWall"));
    EastWall->SetupAttachment(SpawnArea);
    EastWall->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    EastWall->SetCollisionResponseToAllChannels(ECR_Ignore);
    EastWall->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    EastWall->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
    EastWall->SetHiddenInGame(true);

    // West Wall
    WestWall = CreateDefaultSubobject<UBoxComponent>(TEXT("WestWall"));
    WestWall->SetupAttachment(SpawnArea);
    WestWall->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    WestWall->SetCollisionResponseToAllChannels(ECR_Ignore);
    WestWall->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    WestWall->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
    WestWall->SetHiddenInGame(true);
}

// Called when the game starts or when spawned
void AFishSpawnManager::BeginPlay()
{
	Super::BeginPlay();
	
    OceanManager = Cast<AOceanManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AOceanManager::StaticClass()));

    if (!ManagerConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("FishSpawnManager: No ManagerConfig"));
        return;
    }

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AFishSpawnManager::TrySpawnSwarms, ManagerConfig->CheckInterval, true);

	TrySpawnSwarms();
}

void AFishSpawnManager::TrySpawnSwarms()
{
    if (!ManagerConfig) return;

    // lambda
    auto ProcessSpawnList = [this](const TArray<FSwarmSpawnInfo>& SpawnList, int32 GlobalMaxLimit, bool bMustBeAtSurface)
        {
            if (SpawnList.Num() == 0) return;

            // calculate current global count and individual counts
            int32 CurrentGlobalCount = 0;
            TArray<int32> CurrentCounts;
            CurrentCounts.Init(0, SpawnList.Num());

            for (int32 i = 0; i < SpawnList.Num(); ++i)
            {
                if (SpawnList[i].SwarmClass)
                {
                    TArray<AActor*> FoundActors;
                    UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnList[i].SwarmClass, FoundActors);
                    CurrentCounts[i] = FoundActors.Num();
                    CurrentGlobalCount += FoundActors.Num();
                }
            }

            // calculate available slots
            int32 AvailableGlobalSlots = GlobalMaxLimit - CurrentGlobalCount;
            if (AvailableGlobalSlots <= 0) return; 

            // Spawn swarm
            auto SpawnSingleSwarm = [this, bMustBeAtSurface](const FSwarmSpawnInfo& Info)
                {
                    //if (Info.ConfigPool.Num() == 0)
                    //{
                    //    UE_LOG(LogTemp, Warning, TEXT("SpawnSingleSwarm: ConfigPool is empty for %s"), *Info.SwarmClass->GetName());
                    //    return;
                    //}

                    //// Roulette Wheel Selection for Config
                    //float TotalWeight = 0.0f;
                    //for (const FWeightedSwarmConfig& WeightedConfig : Info.ConfigPool)
                    //{
                    //    TotalWeight += WeightedConfig.Weight;
                    //}

                    //float RandomPointVal = FMath::FRandRange(0.0f, TotalWeight);
                    //UFishSwarmConfig* SelectedConfig = nullptr;

                    //for (const FWeightedSwarmConfig& WeightedConfig : Info.ConfigPool)
                    //{
                    //    RandomPointVal -= WeightedConfig.Weight;
                    //    if (RandomPointVal <= 0.0f)
                    //    {
                    //        SelectedConfig = WeightedConfig.SwarmConfig;
                    //        break;
                    //    }
                    //}
                    // 
                    //// Fallback just in case
                    //if (!SelectedConfig)
                    //{
                    //    SelectedConfig = Info.ConfigPool.Last().SwarmConfig;
                    //}

                    UFishSwarmConfig* SelectedConfig = nullptr;

                    if (Info.ConfigPool.Num() > 0)
                    {
                        // Roulette Wheel Selection for Config
                        float TotalWeight = 0.0f;
                        for (const FWeightedSwarmConfig& WeightedConfig : Info.ConfigPool)
                        {
                            TotalWeight += WeightedConfig.Weight;
                        }

                        float RandomPointVal = FMath::FRandRange(0.0f, TotalWeight);

                        for (const FWeightedSwarmConfig& WeightedConfig : Info.ConfigPool)
                        {
                            RandomPointVal -= WeightedConfig.Weight;
                            if (RandomPointVal <= 0.0f)
                            {
                                SelectedConfig = WeightedConfig.SwarmConfig;
                                break;
                            }
                        }

                        // Fallback just in case
                        if (!SelectedConfig)
                        {
                            SelectedConfig = Info.ConfigPool.Last().SwarmConfig;
                        }
                    }

                    //generate avoid player
                    FVector RandomPoint = FVector::ZeroVector;
                    bool bValidPoint = false;
                    int32 MaxAttempts = 10;
                    
                    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
                    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;

                    for (int32 i = 0; i < MaxAttempts; ++i)
                    {
                        float EdgeMargin = 500.0f;

                        
                        FVector OriginalExtent = SpawnArea->Bounds.BoxExtent;

                        FVector SafeExtent = OriginalExtent - FVector(EdgeMargin, EdgeMargin, 0.0f);

                        SafeExtent.X = FMath::Max(0.0f, SafeExtent.X);
                        SafeExtent.Y = FMath::Max(0.0f, SafeExtent.Y);
                        SafeExtent.Z = OriginalExtent.Z;

                        RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(SpawnArea->Bounds.Origin, SafeExtent);

                        if (bMustBeAtSurface && OceanManager)
                        {
                            RandomPoint.Z = OceanManager->GetApproximateOceanHeight(RandomPoint);
                        }

                        if (PlayerPawn)
                        {
                            float Dist = FVector::DistXY(RandomPoint, PlayerLocation);
                            if (Dist >= 1000.0f) 
                            {
                                bValidPoint = true;
                                break;
                            }
                        }
                        else
                        {
                            bValidPoint = true;
                            break;
                        }
                    }

                    if (!bValidPoint)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("SpawnSingleSwarm: Failed to find a valid spawn point away from player after 10 attempts."));
                        return;
                    }

                    float RandomScale = FMath::RandRange(Info.RandomScaleRange.X, Info.RandomScaleRange.Y);
                    FTransform SpawnTransform;
                    SpawnTransform.SetLocation(RandomPoint);
                    SpawnTransform.SetScale3D(FVector(RandomScale));

                    FActorSpawnParameters SpawnParams;
                    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Info.SwarmClass, SpawnTransform, SpawnParams);

                    if (SpawnedActor)
                    {
                        if (AFishSwarmActor* SwarmActor = Cast<AFishSwarmActor>(SpawnedActor))
                        {
                            SwarmActor->InitializeSwarm(SelectedConfig);
                        }
                    }
                };


            //MinCount
            for (int32 i = 0; i < SpawnList.Num() && AvailableGlobalSlots > 0; ++i)
            {
                const FSwarmSpawnInfo& Info = SpawnList[i];
                if (!Info.SwarmClass) continue;

                while (CurrentCounts[i] < Info.MinCount && AvailableGlobalSlots > 0)
                {
                    SpawnSingleSwarm(Info);
                    CurrentCounts[i]++;
                    AvailableGlobalSlots--;
                }
            }


            // Random add
            while (AvailableGlobalSlots > 0)
            {
                TArray<int32> ValidIndices;

                for (int32 i = 0; i < SpawnList.Num(); ++i)
                {
                    if (SpawnList[i].SwarmClass && CurrentCounts[i] < SpawnList[i].MaxCount)
                    {
                        ValidIndices.Add(i);
                    }
                }

                if (ValidIndices.Num() == 0) break;

                int32 RandomIndex = ValidIndices[FMath::RandRange(0, ValidIndices.Num() - 1)];
                SpawnSingleSwarm(SpawnList[RandomIndex]);

                CurrentCounts[RandomIndex]++;
                AvailableGlobalSlots--;
            }
        };

    ProcessSpawnList(ManagerConfig->InteractiveSwarmsToSpawn, ManagerConfig->GlobalMaxInteractiveSwarms, true);
    ProcessSpawnList(ManagerConfig->AmbientSwarmsToSpawn, ManagerConfig->GlobalMaxAmbientSwarms, false);
}


// Called every frame
void AFishSpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFishSpawnManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!SpawnArea) return;

	const FVector Extent = SpawnArea->GetUnscaledBoxExtent();
	const float WallThickness = 500.0f;
	const float WallHeight = 20000.0f;

	// North Wall (+X)
	if (NorthWall)
	{
		NorthWall->SetBoxExtent(FVector(WallThickness, Extent.Y, WallHeight));
		NorthWall->SetRelativeLocation(FVector(Extent.X + WallThickness, 0.0f, 0.0f));
	}

	// South Wall (-X)
	if (SouthWall)
	{
		SouthWall->SetBoxExtent(FVector(WallThickness, Extent.Y, WallHeight));
		SouthWall->SetRelativeLocation(FVector(-(Extent.X + WallThickness), 0.0f, 0.0f));
	}

	// East Wall (+Y)
	if (EastWall)
	{
		EastWall->SetBoxExtent(FVector(Extent.X + (WallThickness * 2.0f), WallThickness, WallHeight));
		EastWall->SetRelativeLocation(FVector(0.0f, Extent.Y + WallThickness, 0.0f));
	}

	// West Wall (-Y)
	if (WestWall)
	{
		WestWall->SetBoxExtent(FVector(Extent.X + (WallThickness * 2.0f), WallThickness, WallHeight));
		WestWall->SetRelativeLocation(FVector(0.0f, -(Extent.Y + WallThickness), 0.0f));
	}
}

