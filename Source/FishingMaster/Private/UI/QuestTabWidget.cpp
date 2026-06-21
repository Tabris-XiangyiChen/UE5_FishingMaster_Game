// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/QuestTabWidget.h"
#include "UI/QuestObjectiveEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "FishData.h"

void UQuestTabWidget::UpdateQuestInfo(const FQuestData& QuestData, const TArray<int32>& CurrentProgress)
{
    UE_LOG(LogTemp, Warning, TEXT("UI_Tab: TextBlocks Valid - Title:%d Desc:%d List:%d"), 
        QuestTitleText != nullptr, QuestDescriptionText != nullptr, ObjectiveList != nullptr);

    if (QuestTitleText)
    {
        QuestTitleText->SetText(QuestData.QuestTitle);
    }

    if (QuestDescriptionText)
    {
        QuestDescriptionText->SetText(QuestData.QuestDescription);
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
                FFishSpeciesData* FishData = Objective.TargetFish.GetRow<FFishSpeciesData>(TEXT("QuestTab_IconLookup"));
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
                ResolvedName = TEXT("Any Fish");
                ResolvedIcon = QuestData.DefaultObjectiveIcon;
            }

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
