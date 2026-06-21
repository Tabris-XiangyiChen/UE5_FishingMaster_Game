// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestObjectiveEntryWidget.generated.h"

class UTextBlock;
class UImage;

/**
 *
 */
UCLASS()
class FISHINGMASTER_API UQuestObjectiveEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Objective name Optional
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ObjectiveNameText;

    // Progress
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ObjectiveProgressText;

    // Icon
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ObjectiveIcon;

    UFUNCTION(BlueprintCallable, Category = "Objective Entry")
    void SetupObjective(const FString& Name, const FString& Progress, UTexture2D* Icon);
};
