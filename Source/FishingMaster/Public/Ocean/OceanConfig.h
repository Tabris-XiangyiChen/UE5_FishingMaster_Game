// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OceanManager.h"
#include "OceanConfig.generated.h"

// Per-Cascade Parameters Struct
USTRUCT(BlueprintType)
struct FOceanCascadeParams
{
    GENERATED_BODY()

    //The size of the surface in the physical world of this cascade£¨m£©¡£
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cascade", meta = (ClampMin = "1.0"))
    float PatchLength = 100.0f;

    // The cascaded wave intensity amplitude
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cascade", meta = (ClampMin = "0.0"))
    float WaveAmplitude = 1.0f;

    // The horizontal compression factor of this cascaded layer can be increased to generate sharp wave crests, 
    // while the low-frequency giant wave factor should be decreased to prevent grid self-crossing.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cascade", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float ChoppyScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cascade")
    float WindAngleOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cascade", meta = (ClampMin = "0.0"))
    float CascadeWindSpeed = 15.0f;
};

/**
 * Global Ocean Configuration Data Asset
 */

UCLASS()
class FISHINGMASTER_API UOceanConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

    // Phillips / JONSWAP
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment|Weather")
    EOceanSpectrumType SpectrumModel = EOceanSpectrumType::JONSWAP;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment|Weather", meta = (ClampMin = "0.0"))
    float WindSpeed = 15.0f;

    // Wind dir
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment|Weather")
    FVector2D WindDirection = FVector2D(1.0f, 1.0f);


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment|Weather", meta = (ClampMin = "0.0001", ClampMax = "0.1"))
    float WindDependency = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment|Weather|JONSWAP", meta = (EditCondition = "SpectrumModel == EOceanSpectrumType::JONSWAP", ClampMin = "1.0"))
    float Fetch = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment|Weather|JONSWAP", meta = (EditCondition = "SpectrumModel == EOceanSpectrumType::JONSWAP", ClampMin = "1.0", ClampMax = "10.0"))
    float JONSWAP_Gamma = 5.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cascades", meta = (TitleProperty = "PatchLength"))
    TArray<FOceanCascadeParams> CascadePresets;
	
};
