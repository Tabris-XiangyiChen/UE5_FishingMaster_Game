// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/QuestTrackerWidget.h"
#include "UI/QuestObjectiveEntryWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "FishData.h"

void UQuestTrackerWidget::UpdateTracker(const FQuestData& QuestData, const TArray<int32>& CurrentProgress)
{
    // Update Title
    if (QuestTitleText)
    {
        QuestTitleText->SetText(QuestData.QuestTitle);
    }

    // Update Main Icon
    if (QuestMainIconImage)
    {
        if (QuestData.QuestMainIcon)
        {
            QuestMainIconImage->SetBrushFromTexture(QuestData.QuestMainIcon);
            QuestMainIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
        else
        {
            QuestMainIconImage->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // Rebuild objective entry list
    if (ObjectiveList && ObjectiveEntryClass)
    {
        ObjectiveList->ClearChildren();

        for (int32 i = 0; i < QuestData.Objectives.Num(); ++i)
        {
            const FQuestObjective& Objective = QuestData.Objectives[i];
            int32 CurProg = CurrentProgress.IsValidIndex(i) ? CurrentProgress[i] : 0;
            int32 ReqAmount = Objective.RequiredAmount;

            FString ResolvedName;
            UTexture2D* ResolvedIcon = nullptr;

            if (Objective.ObjectiveType == EQuestObjectiveType::CatchSpecificFish && !Objective.TargetFish.IsNull())
            {
                ResolvedName = Objective.TargetFish.RowName.ToString();
                FFishSpeciesData* FishData = Objective.TargetFish.GetRow<FFishSpeciesData>(TEXT("QuestTracker_IconLookup"));
                if (FishData)
                {
                    ResolvedIcon = FishData->FishIcon;
                }
            }
            else if (Objective.ObjectiveType == EQuestObjectiveType::CollectAnyBombedFish)
            {
                ResolvedName = TEXT("Bombed Fish");
                ResolvedIcon = QuestData.DefaultObjectiveIcon;
            }
            else
            {
                // CatchAnyFish
                ResolvedName = TEXT("Any Fish");
                ResolvedIcon = QuestData.DefaultObjectiveIcon;
            }

            // Create entry and add to list
            UQuestObjectiveEntryWidget* Entry = CreateWidget<UQuestObjectiveEntryWidget>(this, ObjectiveEntryClass);
            if (Entry)
            {
                FString ProgressStr = FString::Printf(TEXT("%d/%d"), CurProg, ReqAmount);
                Entry->SetupObjective(ResolvedName, ProgressStr, ResolvedIcon);
                ObjectiveList->AddChildToVerticalBox(Entry);
            }
        }
    }
}
