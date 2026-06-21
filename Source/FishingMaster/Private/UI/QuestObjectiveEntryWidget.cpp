// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/QuestObjectiveEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UQuestObjectiveEntryWidget::SetupObjective(const FString& Name, const FString& Progress, UTexture2D* Icon)
{
    if (ObjectiveNameText)
    {
        ObjectiveNameText->SetText(FText::FromString(Name));
    }

    // rogress text
    if (ObjectiveProgressText)
    {
        ObjectiveProgressText->SetText(FText::FromString(Progress));
    }

    // Icon
    if (ObjectiveIcon)
    {
        if (Icon)
        {
            ObjectiveIcon->SetBrushFromTexture(Icon);
            ObjectiveIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
        else
        {
            ObjectiveIcon->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}
