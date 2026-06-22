// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestManagerComponent.h"

UQuestManagerComponent::UQuestManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UQuestManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    // Auto-assign first quest
    if (InitialQuestChain.Num() > 0)
    {
        AddQuest(InitialQuestChain[0]);
    }
}

void UQuestManagerComponent::AddQuest(UQuestConfig* NewQuest)
{
    if (!NewQuest) return;

    for (const FActiveQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestConfig == NewQuest) return;
    }

    FActiveQuest NewActiveQuest;
    NewActiveQuest.QuestConfig = NewQuest;
    NewActiveQuest.ObjectiveProgress.Init(0, NewQuest->QuestData.Objectives.Num());

    ActiveQuests.Add(NewActiveQuest);

    // Broadcast initial state
    OnQuestProgressUpdated.Broadcast(NewQuest->QuestData, NewActiveQuest.ObjectiveProgress);
}

void UQuestManagerComponent::ReportAction(FName FishRowName, int32 Count, bool bIsBombedCollection)
{
    // Reverse iteration to support safe removal
    for (int32 i = ActiveQuests.Num() - 1; i >= 0; --i)
    {
        FActiveQuest& ActiveQuest = ActiveQuests[i];
        if (!ActiveQuest.QuestConfig) continue;

        const FQuestData& QData = ActiveQuest.QuestConfig->QuestData;
        bool bProgressChanged = false;
        bool bAllObjectivesComplete = true;

        // Iterate through all objectives
        for (int32 ObjIndex = 0; ObjIndex < QData.Objectives.Num(); ++ObjIndex)
        {
            const FQuestObjective& Objective = QData.Objectives[ObjIndex];
            
            // Skip if this objective is already complete
            if (ActiveQuest.ObjectiveProgress[ObjIndex] >= Objective.RequiredAmount)
            {
                continue;
            }

            bool bObjectiveMatched = false;

            switch (Objective.ObjectiveType)
            {
                case EQuestObjectiveType::CatchSpecificFish:
                    if (Objective.TargetFish.RowName == FishRowName)
                    {
                        bObjectiveMatched = true;
                    }
                    break;
                case EQuestObjectiveType::CatchAnyFish:
                    if (!bIsBombedCollection)
                    {
                        bObjectiveMatched = true;
                    }
                    break;
                case EQuestObjectiveType::CollectAnyBombedFish:
                    if (bIsBombedCollection)
                    {
                        bObjectiveMatched = true;
                    }
                    break;
            }

            if (bObjectiveMatched)
            {
                ActiveQuest.ObjectiveProgress[ObjIndex] += Count;
                if (ActiveQuest.ObjectiveProgress[ObjIndex] > Objective.RequiredAmount)
                {
                    ActiveQuest.ObjectiveProgress[ObjIndex] = Objective.RequiredAmount;
                }
                bProgressChanged = true;
            }

            // Check if THIS objective is complete after potential update
            if (ActiveQuest.ObjectiveProgress[ObjIndex] < Objective.RequiredAmount)
            {
                bAllObjectivesComplete = false;
            }
        }

        if (bProgressChanged)
        {
            //  Broadcast progress update
            OnQuestProgressUpdated.Broadcast(QData, ActiveQuest.ObjectiveProgress);

            // Check complete entire quest
            if (bAllObjectivesComplete)
            {
                OnQuestCompleted.Broadcast(QData);
                //  Remove
                ActiveQuests.RemoveAt(i);

                CurrentChainIndex++;
                if (CurrentChainIndex < InitialQuestChain.Num())
                {
                    AddQuest(InitialQuestChain[CurrentChainIndex]);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("QuestManager: All quests in chain completed!"));
                }
            }
        }
    }
}
