// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LootNotificationContainer.generated.h"

class UVerticalBox;
class ULootNotificationItemWidget;
class UTexture2D;

/**
 *
 */
UCLASS()
class FISHINGMASTER_API ULootNotificationContainer : public UUserWidget
{
    GENERATED_BODY()

public:
    //Add a new loot notification
    UFUNCTION(BlueprintCallable, Category = "Loot UI")
    void AddLootNotification(const FText& FishName, int32 Count, UTexture2D* Icon = nullptr);

    // Max  notifications
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot UI")
    int32 MaxNotifications = 5;

    // Notification Item Widget Class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot UI")
    TSubclassOf<ULootNotificationItemWidget> NotificationItemClass;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> NotificationList;
};
