// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "FishSwarmConfig.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FFishSpawnWeight
{
    GENERATED_BODY()

    // Fish table
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
    FDataTableRowHandle FishRow;

    // Weight , 0 - 100
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.1"))
    float Weight = 10.0f;
};

/**
 * 
 */
UCLASS()
class FISHINGMASTER_API UFishSwarmConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    // Fish type pool
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pool")
    TArray<FFishSpawnWeight> FishPool;

    // Max fish num
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Capacity", meta = (ClampMin = "1"))
    int32 MaxCapacity = 15;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Swarm Status")
    float SwarmRadius = 500.0f;

    // Boob fish num
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bombing")
    FVector2D BombYieldRange = FVector2D(5.0f, 12.0f);

    // VFX
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TObjectPtr<UNiagaraSystem> UnderwaterSwarmVFX;

    // Value Indicator
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size & Visuals|Indicator")
    TObjectPtr<UStaticMesh> ValueIndicatorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size & Visuals|Indicator")
    float IndicatorRotationSpeed = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = " Size & Visuals|Indicator")
    float IndicatorBobbingSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Size & Visuals|Indicator")
    float IndicatorBobbingHeight = 20.0f;

    // Hooking time scaling factor (base time 10 seconds, factor 0.5 becomes 5 seconds)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Modifier")
    float BiteTimeMultiplierMin = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Modifier")
    float BiteTimeMultiplierMax = 0.7f;
	
};
