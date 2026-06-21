#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FishingPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UQuestManagerComponent;
class UGlobalAudioConfig;
class UAudioComponent;
class UPauseMenuWidget;

/**
 * 
 */
UCLASS()
class FISHINGMASTER_API AFishingPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AFishingPlayerController();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Input")
    TObjectPtr<UInputMappingContext> UIMappingContext;

    // Input action to toggle Quest UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Input")
    TObjectPtr<UInputAction> ToggleQuestUIAction;

    // Input action to toggle Pause Menu
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Input")
    TObjectPtr<UInputAction> PauseAction;

    // Pause Menu Widget class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Input")
    TSubclassOf<UPauseMenuWidget> PauseMenuClass;

protected:
    virtual void BeginPlay() override;

    virtual void SetupInputComponent() override;

    // Toggle the Quest UI and manage input modes
    void ToggleQuestUI();

    // Toggle the Pause Menu
    void TogglePauseMenu();

    // Keep track of whether the UI is open
    bool bIsQuestUIOpen = false;

    // Keep track of whether the game is paused
    bool bIsPaused = false;

    // Pause Menu Widget instance
    UPROPERTY()
    TObjectPtr<UPauseMenuWidget> PauseMenuWidget;

public:
    // Quest Manager
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TObjectPtr<UQuestManagerComponent> QuestManager;

    // Global Audio Config
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<UGlobalAudioConfig> GlobalAudioConfig;

    // BGM Audio Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<UAudioComponent> BGMAudioComponent;

    // Seagull sound timer handle
    FTimerHandle SeagullTimerHandle;
};