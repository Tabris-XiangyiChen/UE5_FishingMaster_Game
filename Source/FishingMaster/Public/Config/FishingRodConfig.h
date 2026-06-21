// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/NoExportTypes.h"
#include "FishingRodConfig.generated.h"

/**
 * 
 */
UCLASS()
class FISHINGMASTER_API UFishingRodConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    float RaiseSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    float LowerSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    float RodMaxAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    float ResetWaitTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bite Logic")
    float BiteWaitTimeMin = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bite Logic")
    float BiteWaitTimeMax = 10.0f;

    // Try use new ue struct
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bite Logic")
    FFloatInterval FeverBiteTime = { 0.2f, 0.5f };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE System")
    float QTEWindowNormal = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE System")
    float QTEWindowFever = 2.0f;
;

    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE System", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    //float QTEPerfectRangeMin = 0.7f;

    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE System", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    //float QTEPerfectRangeMax = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Feedback")
    float ShakeFrequency = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Feedback")
    float ShakeAmplitude = 3.0f;

    // --- Audio ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> FishingCastSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> FishingReelSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> EvalPerfectSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> EvalGoodSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> EvalMissSound;
	
};
