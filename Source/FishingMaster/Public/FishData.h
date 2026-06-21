#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "FishVisual.h"
#include "FishData.generated.h"


UENUM(BlueprintType)
enum class EFishRarity : uint8
{
    Common      UMETA(DisplayName = "Common (White)"),
    Uncommon    UMETA(DisplayName = "Uncommon (Green)"),
    Rare        UMETA(DisplayName = "Rare (Blue)"),
    Epic        UMETA(DisplayName = "Epic (Purple)"),
    Legendary   UMETA(DisplayName = "Legendary (Gold)")
};

USTRUCT(BlueprintType)
struct FISHINGMASTER_API FFishSpeciesData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // Name
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    FText FishName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    FText Description;

    // Catch visual class
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSubclassOf<AFishCaughtVisual> CaughtFishClass;

    // Bomb visual class
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSubclassOf<AFishBombedVisual> BombedFishClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals|UI")
    TObjectPtr<class UTexture2D> FishIcon; 

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> FishCollectSound;

    // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    // TObjectPtr<USkeletalMesh> FishModel; 

    // worth and weight
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    EFishRarity Rarity = EFishRarity::Common;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float BasePrice = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    FVector2D WeightRange = FVector2D(1.0f, 5.0f);

    // Gameplay states
    // The total number of seconds it takes for the entire QTE progress bar to go from 0% to 100%.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE Timing")
    float QTETotalDuration = 5.0f;

    // The percentage of the catch zone's startpoint
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE Timing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CatchStartRatio = 0.4f;

    // The percentage of the catch zone's endpoint
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE Timing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CatchEndRatio = 0.7f;

    // Perfect Zone start, int the catch zone
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE Timing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PerfectStartRatio = 0.52f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE Timing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PerfectEndRatio = 0.58f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    float StruggleIntensity = 1.0f;
};