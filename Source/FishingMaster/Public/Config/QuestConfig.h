// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "QuestConfig.generated.h"

UENUM(BlueprintType)
enum class EQuestObjectiveType : uint8
{
    CatchSpecificFish UMETA(DisplayName = "Catch Specific Fish"),
    CatchAnyFish UMETA(DisplayName = "Catch Any Fish (Rod)"),
    CollectAnyBombedFish UMETA(DisplayName = "Collect Any Bombed Fish")
};

USTRUCT(BlueprintType)
struct FQuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    EQuestObjectiveType ObjectiveType = EQuestObjectiveType::CatchSpecificFish;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FDataTableRowHandle TargetFish; // Used only if CatchSpecificFish

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective", meta=(ClampMin="1"))
    int32 RequiredAmount = 1;
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FISHINGMASTER_API FQuestData
{
    GENERATED_BODY()

public:
    // Quest Title
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    //Target Fish
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuestObjective> Objectives;

    // Completion Text
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText CompletionText;

    // Main icon displayed in the Quest Tracker HUD
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|UI")
    TObjectPtr<class UTexture2D> QuestMainIcon;

    // Fallback icon for generic objectives (Catch Any / Bombed Fish)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|UI")
    TObjectPtr<class UTexture2D> DefaultObjectiveIcon;

    // Completion Icon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TObjectPtr<class UTexture2D> CompletionIcon;
};

/**
 *
 */
UCLASS()
class FISHINGMASTER_API UQuestConfig : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuestData QuestData;
};
