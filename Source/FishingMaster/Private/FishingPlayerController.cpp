#include "FishingPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UI/FishingHUD.h"
#include "UI/PauseMenuWidget.h"
#include "QuestManagerComponent.h"
#include "Config/GlobalAudioConfig.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

AFishingPlayerController::AFishingPlayerController()
{
    QuestManager = CreateDefaultSubobject<UQuestManagerComponent>(TEXT("QuestManager"));
}

void AFishingPlayerController::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

    if (GlobalAudioConfig)
    {
        // Global BGM
        if (GlobalAudioConfig->GlobalBGM)
        {
            BGMAudioComponent = UGameplayStatics::SpawnSound2D(this, GlobalAudioConfig->GlobalBGM);
            if (BGMAudioComponent)
            {
                BGMAudioComponent->Play();
            }
        }

        // Ocean Ambience
        if (GlobalAudioConfig->OceanAmbienceSound)
        {
            UGameplayStatics::SpawnSound2D(this, GlobalAudioConfig->OceanAmbienceSound);
        }

        // Seagull Timer
        if (GlobalAudioConfig->SeagullSound)
        {
            GetWorldTimerManager().SetTimer(SeagullTimerHandle, [this]()
            {
                if (GlobalAudioConfig && GlobalAudioConfig->SeagullSound)
                {
                    UGameplayStatics::PlaySound2D(this, GlobalAudioConfig->SeagullSound);
                }
            }, FMath::RandRange(8.0f, 20.0f), true, FMath::RandRange(0.0f, 5.0f));
        }
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(UIMappingContext, 1);
    }
}

void AFishingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (ToggleQuestUIAction)
        {
            EnhancedInputComponent->BindAction(ToggleQuestUIAction, ETriggerEvent::Started, this, &AFishingPlayerController::ToggleQuestUI);
        }

        if (PauseAction)
        {
            EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &AFishingPlayerController::TogglePauseMenu);
        }
    }
}

void AFishingPlayerController::ToggleQuestUI()
{
    if (AFishingHUD* FishingHUD = Cast<AFishingHUD>(GetHUD()))
    {
        bIsQuestUIOpen = FishingHUD->ToggleQuestTab();

        if (bIsQuestUIOpen)
        {
            // Set Input Mode to Game And UI, show mouse cursor
            FInputModeGameAndUI InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            InputMode.SetHideCursorDuringCapture(false);
            SetInputMode(InputMode);
            bShowMouseCursor = true;
        }
        else
        {
            // Revert Input Mode to Game Only, hide mouse cursor
            FInputModeGameOnly InputMode;
            SetInputMode(InputMode);
            bShowMouseCursor = false;
        }
    }
}

void AFishingPlayerController::TogglePauseMenu()
{
    if (!PauseMenuClass) return;

    if (!bIsPaused)
    {
        // Pause the game
        PauseMenuWidget = CreateWidget<UPauseMenuWidget>(this, PauseMenuClass);
        if (PauseMenuWidget)
        {
            PauseMenuWidget->AddToViewport(10);

            UGameplayStatics::SetGamePaused(this, true);

            FInputModeGameAndUI InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            InputMode.SetHideCursorDuringCapture(false);
            SetInputMode(InputMode);
            bShowMouseCursor = true;

            bIsPaused = true;
        }
    }
    else
    {
        // Resume
        if (PauseMenuWidget)
        {
            PauseMenuWidget->RemoveFromParent();
            PauseMenuWidget = nullptr;
        }

        UGameplayStatics::SetGamePaused(this, false);

        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = false;

        bIsPaused = false;
    }
}