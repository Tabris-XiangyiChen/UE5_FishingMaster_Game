// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FishManagerConfig.generated.h"

USTRUCT(BlueprintType)
struct FWeightedSwarmConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<class UFishSwarmConfig> SwarmConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Weight = 1.0f;
};

USTRUCT(BlueprintType)
struct FSwarmSpawnInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AActor> SwarmClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FWeightedSwarmConfig> ConfigPool;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MinCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector2D RandomScaleRange = FVector2D(0.8f, 1.5f);
};

/**
 * 
 */
UCLASS()
class FISHINGMASTER_API UFishManagerConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Settings")
    float CheckInterval = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive Swarms")
    int32 GlobalMaxInteractiveSwarms = 20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive Swarms")
    TArray<FSwarmSpawnInfo> InteractiveSwarmsToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient Swarms")
    int32 GlobalMaxAmbientSwarms = 15;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient Swarms")
    TArray<FSwarmSpawnInfo> AmbientSwarmsToSpawn;
};
