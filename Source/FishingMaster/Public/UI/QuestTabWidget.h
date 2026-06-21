// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Config/QuestConfig.h"
#include "QuestTabWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UQuestObjectiveEntryWidget;

/**
 *
 */
UCLASS()
class FISHINGMASTER_API UQuestTabWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Quest UI")
    void UpdateQuestInfo(const FQuestData& QuestData, const TArray<int32>& CurrentProgress);

protected:
    // Quest title
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> QuestTitleText;

    // Quest description
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> QuestDescriptionText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> ObjectiveList;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UQuestObjectiveEntryWidget> ObjectiveEntryClass;
};
