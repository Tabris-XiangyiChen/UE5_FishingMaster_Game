// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Config/QuestConfig.h"
#include "FishingHUD.generated.h"

class UQuestTrackerWidget;
class ULootNotificationContainer;
class UQuestTabWidget;
class UControlsHintWidget;

/**
 *
 */
UCLASS()
class FISHINGMASTER_API AFishingHUD : public AHUD
{
    GENERATED_BODY()

public:
    AFishingHUD();

protected:
    virtual void BeginPlay() override;

public:

    UFUNCTION(BlueprintCallable, Category = "Fishing HUD")
    void ShowLootNotification(const FText& FishName, int32 Count, class UTexture2D* Icon = nullptr);

    // Update the HUD quest tracker
    UFUNCTION(BlueprintCallable, Category = "Fishing HUD")
    void UpdateQuestTracker(const FQuestData& QuestData, const TArray<int32>& CurrentProgress);

    UFUNCTION(BlueprintCallable, Category = "Fishing HUD")
    bool ToggleQuestTab();

    UFUNCTION()
    void HandleQuestProgressUpdated(const FQuestData& QuestData, const TArray<int32>& CurrentProgress);

    UFUNCTION()
    void HandleQuestCompleted(const FQuestData& QuestData);

protected:
    // Quest Tab Widget Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing HUD|Classes")
    TSubclassOf<UQuestTabWidget> QuestTabClass;

    // Quest Tracker
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing HUD|Classes")
    TSubclassOf<UQuestTrackerWidget> QuestTrackerClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing HUD|Classes")
    TSubclassOf<UControlsHintWidget> ControlsHintClass;

    //Loot Notification Container
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing HUD|Classes")
    TSubclassOf<ULootNotificationContainer> LootContainerClass;

    // Active Quest Tracker Instance
    UPROPERTY(Transient)
    TObjectPtr<UQuestTrackerWidget> QuestTrackerWidget;


    UPROPERTY(Transient)
    TObjectPtr<ULootNotificationContainer> LootContainerWidget;

    UPROPERTY(Transient)
    TObjectPtr<UQuestTabWidget> QuestTabWidget;

    UPROPERTY(Transient)
    TObjectPtr<UControlsHintWidget> ControlsHintWidget;

    // Quests State Cache
    FQuestData CachedQuestData;
    TArray<int32> CachedProgress;
    bool bHasCachedQuest = false;
};
