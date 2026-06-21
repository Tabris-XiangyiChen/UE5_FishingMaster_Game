#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GlobalAudioConfig.generated.h"

class USoundBase;

/**
 *
 */
UCLASS()
class FISHINGMASTER_API UGlobalAudioConfig : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> GlobalBGM;

    // Ocean Ambience
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Environment")
    TObjectPtr<USoundBase> OceanAmbienceSound;

    // Seagull Cry
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Environment")
    TObjectPtr<USoundBase> SeagullSound;
};
