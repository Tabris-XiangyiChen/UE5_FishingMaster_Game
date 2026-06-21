// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Config/QuestConfig.h"
#include "QuestManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestProgressUpdated, const FQuestData&, QuestData, const TArray<int32>&, CurrentProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FQuestData&, QuestData);

class UQuestConfig;

/**
 * 
 */
USTRUCT(BlueprintType)
struct FISHINGMASTER_API FActiveQuest
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TObjectPtr<UQuestConfig> QuestConfig = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<int32> ObjectiveProgress;
};

/**
 *
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FISHINGMASTER_API UQuestManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:	
    UQuestManagerComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Initial Quest Sequence
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Sequence")
    TArray<TObjectPtr<UQuestConfig>> InitialQuestChain;

    // Current Chain
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Sequence")
    int32 CurrentChainIndex = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FActiveQuest> ActiveQuests;

    // Quest  update
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestProgressUpdated OnQuestProgressUpdated;

    // Quest completed
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestCompleted OnQuestCompleted;

    // Add quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AddQuest(UQuestConfig* NewQuest);

    //Report action and update quest progress
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportAction(FName FishRowName, int32 Count, bool bIsBombedCollection);
};
