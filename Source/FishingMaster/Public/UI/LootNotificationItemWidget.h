// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LootNotificationItemWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

/**
 *
 */
UCLASS()
class FISHINGMASTER_API ULootNotificationItemWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Loot UI")
    void SetupNotification(const FText& FishName, int32 Count, UTexture2D* Icon = nullptr);

    UFUNCTION(BlueprintImplementableEvent, Category = "Loot UI")
    void PlayFadeOutAnimation();

protected:
    // Fish icon
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> FishIconImage;

    //  Fish name
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> FishNameText;

    // Count
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CountText;
};
