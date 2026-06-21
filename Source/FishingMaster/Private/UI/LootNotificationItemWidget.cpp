// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LootNotificationItemWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void ULootNotificationItemWidget::SetupNotification(const FText& FishName, int32 Count, UTexture2D* Icon)
{
    if (FishNameText)
    {
        FishNameText->SetText(FishName);
    }

    if (CountText)
    {
        if (Count <= 0)
        {
            CountText->SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            FString CountString = FString::Printf(TEXT("x %d"), Count);
            CountText->SetText(FText::FromString(CountString));
            CountText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
    }

    if (FishIconImage && Icon)
    {
        FishIconImage->SetBrushFromTexture(Icon);
    }

    UE_LOG(LogTemp, Warning, TEXT("UI_Item: Setup complete. IconValid: %s"), Icon ? TEXT("True") : TEXT("False"));
}
