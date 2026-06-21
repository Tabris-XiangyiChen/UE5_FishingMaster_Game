// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Config/QuestConfig.h"
#include "QuestTrackerWidget.generated.h"

class UTextBlock;
class UImage;
class UVerticalBox;
class UQuestObjectiveEntryWidget;

/**
 *
 */
UCLASS()
class FISHINGMASTER_API UQuestTrackerWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Quest UI")
    void UpdateTracker(const FQuestData& QuestData, const TArray<int32>& CurrentProgress);

protected:
    // Title
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> QuestTitleText;

    // Main Quest Icon
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> QuestMainIconImage;

    // Objective list
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> ObjectiveList;

    // Objective entry widget class
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UQuestObjectiveEntryWidget> ObjectiveEntryClass;
};
