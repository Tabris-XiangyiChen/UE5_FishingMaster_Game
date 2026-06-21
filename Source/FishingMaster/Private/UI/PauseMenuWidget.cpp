#include "UI/PauseMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "FishingPlayerController.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	}
	if (ReturnToMenuButton)
	{
		ReturnToMenuButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnReturnMenuClicked);
	}
}

void UPauseMenuWidget::OnResumeClicked()
{
	RemoveFromParent();

	UGameplayStatics::SetGamePaused(this, false);

	if (AFishingPlayerController* PC = Cast<AFishingPlayerController>(GetOwningPlayer()))
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

void UPauseMenuWidget::OnReturnMenuClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, FName("MainMenuLevel"));
}
