// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/FishingHUD.h"
#include "UI/QuestTrackerWidget.h"
#include "UI/LootNotificationContainer.h"
#include "UI/QuestTabWidget.h"
#include "Blueprint/UserWidget.h"
#include "FishingPlayerController.h"
#include "QuestManagerComponent.h"
#include "UI/ControlsHintWidget.h"

AFishingHUD::AFishingHUD()
{
}

void AFishingHUD::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = GetOwningPlayerController())
    {
        if (AFishingPlayerController* FPC = Cast<AFishingPlayerController>(PC))
        {
            if (FPC->QuestManager)
            {
                FPC->QuestManager->OnQuestProgressUpdated.AddDynamic(this, &AFishingHUD::HandleQuestProgressUpdated);
                FPC->QuestManager->OnQuestCompleted.AddDynamic(this, &AFishingHUD::HandleQuestCompleted);
            }
        }

        if (QuestTrackerClass)
        {
            QuestTrackerWidget = CreateWidget<UQuestTrackerWidget>(PC, QuestTrackerClass);
            if (QuestTrackerWidget)
            {
                QuestTrackerWidget->AddToViewport();
            }
        }

        // Controls Hint
        if (ControlsHintClass)
        {
            ControlsHintWidget = CreateWidget<UControlsHintWidget>(PC, ControlsHintClass);
            if (ControlsHintWidget)
            {
                ControlsHintWidget->AddToViewport(-1); // -1 Z-Order to stay safely in background
            }
        }

        // Instantiate and add Loot Container to viewport
        if (LootContainerClass)
        {
            LootContainerWidget = CreateWidget<ULootNotificationContainer>(PC, LootContainerClass);
            if (LootContainerWidget)
            {
                LootContainerWidget->AddToViewport();
                UE_LOG(LogTemp, Warning, TEXT("HUD: LootContainer initialized and added to viewport."));
            }
        }
        // Manually pull current state after init to solve lifecycle race condition
        if (AFishingPlayerController* FPC = Cast<AFishingPlayerController>(PC))
        {
            if (FPC->QuestManager && FPC->QuestManager->ActiveQuests.Num() > 0)
            {
                const FActiveQuest& ActiveQuest = FPC->QuestManager->ActiveQuests[0];
                if (ActiveQuest.QuestConfig)
                {
                    CachedQuestData = ActiveQuest.QuestConfig->QuestData;
                    CachedProgress = ActiveQuest.ObjectiveProgress;
                    bHasCachedQuest = true;

                    HandleQuestProgressUpdated(CachedQuestData, CachedProgress);
                }
            }
        }
    }
}

void AFishingHUD::ShowLootNotification(const FText& FishName, int32 Count, UTexture2D* Icon)
{
    UE_LOG(LogTemp, Warning, TEXT("HUD: Routing loot [%s] to Container. Container Valid: %s"), *FishName.ToString(), LootContainerWidget ? TEXT("True") : TEXT("False"));

    if (LootContainerWidget)
    {
        LootContainerWidget->AddLootNotification(FishName, Count, Icon);
    }
}

void AFishingHUD::UpdateQuestTracker(const FQuestData& QuestData, const TArray<int32>& CurrentProgress)
{
    if (QuestTrackerWidget)
    {
        QuestTrackerWidget->UpdateTracker(QuestData, CurrentProgress);
    }
}

void AFishingHUD::HandleQuestProgressUpdated(const FQuestData& QuestData, const TArray<int32>& CurrentProgress)
{
    // Update cache
    CachedQuestData = QuestData;
    CachedProgress = CurrentProgress;
    bHasCachedQuest = true;

    UpdateQuestTracker(QuestData, CurrentProgress);

    UE_LOG(LogTemp, Warning, TEXT("HUD: Delegate received. Pushing update to QuestTab. Tab Valid: %s"), QuestTabWidget ? TEXT("True") : TEXT("False"));

    if (QuestTabWidget)
    {
        QuestTabWidget->UpdateQuestInfo(QuestData, CurrentProgress);
    }
}

void AFishingHUD::HandleQuestCompleted(const FQuestData& QuestData)
{
    // Fix: Show the Data-Driven Completion Text and Icon
    ShowLootNotification(QuestData.CompletionText, 0, QuestData.CompletionIcon);
}

bool AFishingHUD::ToggleQuestTab()
{
    APlayerController* PC = GetOwningPlayerController();
    if (!PC || !QuestTabClass) return false;

    if (!QuestTabWidget)
    {
        QuestTabWidget = CreateWidget<UQuestTabWidget>(PC, QuestTabClass);
        UE_LOG(LogTemp, Warning, TEXT("HUD: Lazy instantiated QuestTabWidget."));
    }

    if (QuestTabWidget)
    {
        if (QuestTabWidget->IsInViewport())
        {
            QuestTabWidget->RemoveFromParent();
            
            // tab close tracker
            if (QuestTrackerWidget)
            {
                QuestTrackerWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            }
            return false;
        }
        else
        {
            // open
            if (QuestTrackerWidget)
            {
                QuestTrackerWidget->SetVisibility(ESlateVisibility::Collapsed);
            }

            QuestTabWidget->AddToViewport();
            
            UE_LOG(LogTemp, Warning, TEXT("HUD: QuestTab toggled. Attempting to push current QuestManager state to Tab."));
            
            // Self-Correction: Manually pull from Cached state to initialize the newly lazy-created widget
            if (bHasCachedQuest)
            {
                QuestTabWidget->UpdateQuestInfo(CachedQuestData, CachedProgress);
            }
            return true;
        }
    }

    return false;
}
