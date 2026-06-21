// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LootNotificationContainer.h"
#include "UI/LootNotificationItemWidget.h"
#include "Components/VerticalBox.h"

void ULootNotificationContainer::AddLootNotification(const FText& FishName, int32 Count, UTexture2D* Icon)
{
    UE_LOG(LogTemp, Warning, TEXT("UI_Container: Spawning Item Widget for [%s]."), *FishName.ToString());

    if (!NotificationList || !NotificationItemClass)
    {
        return;
    }


    ULootNotificationItemWidget* NewNotification = CreateWidget<ULootNotificationItemWidget>(GetWorld(), NotificationItemClass);
    if (NewNotification)
    {

        NewNotification->SetupNotification(FishName, Count, Icon);
        NotificationList->AddChild(NewNotification);
        
        UE_LOG(LogTemp, Warning, TEXT("UI_Container: Item added to list. Current List Count: %d"), NotificationList->GetChildrenCount());

        // Trigger the Blueprint animation
        NewNotification->PlayFadeOutAnimation(); 

        if (NotificationList->GetChildrenCount() > MaxNotifications)
        {
            NotificationList->RemoveChildAt(0);
        }
    }
}
