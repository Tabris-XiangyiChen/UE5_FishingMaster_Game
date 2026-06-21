// Fill out your copyright notice in the Description page of Project Settings.


#include "FishingRodActor.h"
#include "Config/FishingRodConfig.h"
#include "FishData.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFishingRodActor::AFishingRodActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CurrentState = ERodState::Idle;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RodRoot"));
	RootComponent = RootComp;

	RodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RodMesh"));
	RodMesh->SetupAttachment(RootComponent);

	// Create ui component
	QTEWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("QTEWidget"));
	QTEWidgetComponent->SetupAttachment(RootComponent);

	// set invisible
	QTEWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	// default size
	QTEWidgetComponent->SetDrawSize(FVector2D(250, 30.f));
	QTEWidgetComponent->SetVisibility(false);
}

void AFishingRodActor::ActivateRod(int32 InRodIndex)
{
	AssignedIndex = InRodIndex;
	StartLowering();
}

void AFishingRodActor::DeactivateRod()
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	CurrentState = ERodState::Idle;
	//VisualProgress = 0.0f;
	//RodMesh->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	OnQTEStatusChanged.Broadcast(AssignedIndex, false);
	QTEWidgetComponent->SetVisibility(false);
}

void AFishingRodActor::Interact()
{
	//switch (CurrentState)
	//{
	//case ERodState::Idle:
	//{
	//	CurrentState = ERodState::Lowering;
	//	break;
	//}
	//case ERodState::FishOnHook:
	//{
	//	GetWorldTimerManager().ClearTimer(EscapeTimerHandle);
	//	CurrentState = ERodState::Raising;

	//	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("C++: Get Fish"));
	//	break;
	//}
	//case ERodState::WaitingForBite:
	//	GetWorldTimerManager().ClearTimer(BiteTimerHandle);
	//	CurrentState = ERodState::Raising;
	//	break;

	//default:
	//	break;

	//}

	if (CurrentState == ERodState::WaitingForBite)
	{
		GetWorldTimerManager().ClearTimer(BiteTimerHandle);
		OnFishingResult.Broadcast(FName("PUNISH"));
		TriggerMoveToRaising(false, FName("PUNISH"));
		return;
	}

	//if (CurrentState == ERodState::FishOnHook)
	//{
	//	GetWorldTimerManager().ClearTimer(QTEMissTimerHandle);
	//	OnQTEStatusChanged.Broadcast(AssignedIndex, false);

	//	if (bIsFeverActive)
	//	{
	//		// if fever, all perfect
	//		TriggerMoveToRaising(true, FName("PERFECT"));
	//		return;
	//	}


	//	if (QTEProgress >= RodConfig->QTEPerfectRangeMin && QTEProgress <= RodConfig->QTEPerfectRangeMax)
	//	{
	//		TriggerMoveToRaising(true, FName("PERFECT"));
	//	}
	//	else
	//	{
	//		TriggerMoveToRaising(false, FName("MISS"));
	//	}
	//}
}

void AFishingRodActor::SetFeverState(bool bFever)
{
	bIsFeverActive = bFever;

	ToggleFeverUI(bFever);

	if (bFever)
	{
		// change ui
		InitQTEUI(0.0f, 1.0f, 0.0f, 1.0f);

		// clear old state
		if (CurrentState == ERodState::WaitingForBite)
		{
			GetWorldTimerManager().ClearTimer(BiteTimerHandle);

			// Change to fever state, use index to avoid fish bite at the same time
			float StaggeredWaitTime = RodConfig->FeverBiteTime.Min + (AssignedIndex * RodConfig->FeverBiteTime.Max);

			GetWorldTimerManager().SetTimer(BiteTimerHandle, this, &AFishingRodActor::OnFishBite, StaggeredWaitTime, false);
		}
		else if (CurrentState == ERodState::FishOnHook)
		{
			InitQTEUI(0.0f, 1.0f, 0.0f, 1.0f);

			// Caculate remain progess
			float RemainingRatio = 1.0f - QTEProgress;

			// Change qte window time
			QTEWindow = RodConfig->QTEWindowFever;

			// Caculate New ramain time
			float NewRemainingTime = QTEWindow * RemainingRatio;

			// Set new timeout function
			GetWorldTimerManager().SetTimer(QTEMissTimerHandle,
				FTimerDelegate::CreateUObject(this, &AFishingRodActor::TriggerResetToWaiting, FName("TIMEOUT_MISS")),
				NewRemainingTime, false);
		}
	}
	else
	{
		// back to normal
		if (CurrentState == ERodState::FishOnHook && !CurrentLockedFishRow.IsNull())
		{
			FFishSpeciesData* FishData = CurrentLockedFishRow.GetRow<FFishSpeciesData>(TEXT("Restore_UI_Check"));
			if (FishData)
			{
				InitQTEUI(FishData->CatchStartRatio, FishData->CatchEndRatio, FishData->PerfectStartRatio, FishData->PerfectEndRatio);
				

				float RemainingRatio = 1.0f - QTEProgress;

				QTEWindow = FishData->QTETotalDuration;
				// new remain time
				float NewRemainingTime = QTEWindow * RemainingRatio;

				// new set time
				GetWorldTimerManager().SetTimer(QTEMissTimerHandle,
					FTimerDelegate::CreateUObject(this, &AFishingRodActor::TriggerResetToWaiting, FName("TIMEOUT_MISS")),
					NewRemainingTime, false);
			}
		}
	}
}

void AFishingRodActor::SetEnvironmentModifiers(float MinMultiplier, float MaxMultiplier)
{
	CurrentEnvWaitMin = MinMultiplier;
	CurrentEnvWaitMax = MaxMultiplier;
}

void AFishingRodActor::AssignFishData(FDataTableRowHandle FishRowHandle)
{

	if (FishRowHandle.IsNull())
	{
		// Back to waiting, do not call ui
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Silver, TEXT("Nothing happened"));
		//CurrentState = ERodState::Idle;
		//TriggerMoveToRaising(false, FName("NO_FISH"));
		TriggerResetToWaiting(FName("NO_FISH"));
		return;
	}

	CurrentLockedFishRow = FishRowHandle;
	CurrentState = ERodState::FishOnHook;
	QTEProgress = 0.0f;

	if (bIsFeverActive)
	{
		QTEWindow = RodConfig->QTEWindowFever;

		InitQTEUI(0.0f, 1.0f, 0.0f, 1.0f);

		// Broadcast ui
		OnQTEStatusChanged.Broadcast(AssignedIndex, true);
		QTEWidgetComponent->SetVisibility(true);

		GetWorldTimerManager().SetTimer(QTEMissTimerHandle,
			FTimerDelegate::CreateUObject(this, &AFishingRodActor::TriggerMoveToRaising, false, FName("TIMEOUT_MISS")),
			QTEWindow, false);
		
		return;
	}

	// Read DataTable
	FFishSpeciesData* FishData = CurrentLockedFishRow.GetRow<FFishSpeciesData>(TEXT("Context_Bite_Check"));

	//float BaseWindow = RodConfig->QTEWindowNormal;

	if (FishData)
	{

		//QTEWindow = BaseWindow * FishData->QTETotalDuration;
		QTEWindow = FishData->QTETotalDuration;

		// init ui
		InitQTEUI(FishData->CatchStartRatio, FishData->CatchEndRatio, FishData->PerfectStartRatio, FishData->PerfectEndRatio);

		// log
		FString DebugMsg = FString::Printf(TEXT(" A FIsh is biting %d Rod, QTE Time: %.2fs"),
			*FishData->FishName.ToString(), AssignedIndex + 1, QTEWindow);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, DebugMsg);
	}
	else
	{
		QTEWindow = RodConfig->QTEWindowNormal;
	}


	OnQTEStatusChanged.Broadcast(AssignedIndex, true);
	QTEWidgetComponent->SetVisibility(true);
	

	//GetWorldTimerManager().SetTimer(QTEMissTimerHandle, FTimerDelegate::CreateUObject(this, &AFishingRodActor::TriggerMoveToRaising, false, FName("TIMEOUT_MISS")), QTEWindow, false);
	GetWorldTimerManager().SetTimer(QTEMissTimerHandle, FTimerDelegate::CreateUObject(this, &AFishingRodActor::TriggerResetToWaiting, FName("TIMEOUT_MISS")), QTEWindow, false);
}

void AFishingRodActor::TriggerResetToWaiting(FName Evaluation)
{
	OnQTEStatusChanged.Broadcast(AssignedIndex, false);
	QTEWidgetComponent->SetVisibility(false);

	// broadcast to boat
	OnFishingResult.Broadcast(Evaluation);

	CurrentState = ERodState::WaitingForBite;

	if (RodConfig)
	{
		if (Evaluation.ToString().Contains(TEXT("MISS")) || Evaluation == FName("PUNISH"))
		{
			if (RodConfig->EvalMissSound)
			{
				UGameplayStatics::PlaySound2D(this, RodConfig->EvalMissSound);
			}
		}
	}

	// rotate back
	RodMesh->SetRelativeRotation(FRotator(-RodConfig->RodMaxAngle, 0.f, 0.f));

	float RandomBiteTime = 0.0f;

	if (bIsFeverActive)
	{
		RandomBiteTime = FMath::FRandRange(RodConfig->FeverBiteTime.Min, RodConfig->FeverBiteTime.Max);
	}
	else
	{
		float FinalWaitMin = RodConfig->BiteWaitTimeMin * CurrentEnvWaitMin;
		float FinalWaitMax = RodConfig->BiteWaitTimeMax * CurrentEnvWaitMax;
		RandomBiteTime = FMath::FRandRange(FinalWaitMin, FinalWaitMax);
	}

	GetWorldTimerManager().SetTimer(BiteTimerHandle, this, &AFishingRodActor::OnFishBite, RandomBiteTime, false);

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Fish escape, wait for next"));
}

// Called when the game starts or when spawned
void AFishingRodActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (!RodConfig)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("NO Rod config dataasset !"));

		SetActorTickEnabled(false);
		return;
	}
}

//void AFishingRodActor::Lowering(float Time)
//{
//	CurrentProgress += Time * ActionSpeed;
//
//	RodMesh->SetRelativeRotation(FRotator(-45.0f * FMath::Clamp(CurrentProgress, 0.f, 1.f), 0.f, 0.f));
//
//	if (CurrentProgress >= 1.0f)
//	{
//		CurrentState = ERodState::WaitingForBite;
//
//		float RandomWaitTime = FMath::FRandRange(2.0f, 5.0f);
//		GetWorldTimerManager().SetTimer(BiteTimerHandle, this, &AFishingRodActor::OnFishBite, RandomWaitTime, false);
//	}
//}
//
//void AFishingRodActor::Raising(float Time)
//{
//	CurrentProgress -= Time * ActionSpeed;
//	RodMesh->SetRelativeRotation(FRotator(-45.0f * FMath::Clamp(CurrentProgress, 0.f, 1.f), 0.f, 0.f));
//
//	if (CurrentProgress <= 0.0f)
//	{
//		CurrentProgress = 0.0f;
//		CurrentState = ERodState::Idle;
//	}
//}

void AFishingRodActor::StartLowering()
{
	CurrentState = ERodState::Lowering;
	//VisualProgress = 1.0f;

	if (RodConfig && RodConfig->FishingCastSound)
	{
		UGameplayStatics::PlaySound2D(this, RodConfig->FishingCastSound);
	}
}

void AFishingRodActor::OnFishBite()
{

	OnRodRequestFishData.Broadcast(AssignedIndex);

	//CurrentState = ERodState::FishOnHook;
	//QTEProgress = 0.0f;


	//if (bIsFeverActive)
	//{
	//	QTEWindow = RodConfig->QTEWindowFever;
	//}
	//else
	//{
	//	QTEWindow = RodConfig->QTEWindowNormal;
	//	
	//	// broadcast state to UI
	//	OnQTEStatusChanged.Broadcast(AssignedIndex, true);
	//}

	//FString DebugMsg = FString::Printf(TEXT("Number %d Rod hook on bite������"), AssignedIndex + 1);
	//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, DebugMsg);

	//// Times out, Miss!
	//GetWorldTimerManager().SetTimer(QTEMissTimerHandle, FTimerDelegate::CreateUObject(this, &AFishingRodActor::TriggerMoveToRaising, false, FName("MISS")), QTEWindow, false);
}

void AFishingRodActor::TriggerMoveToRaising(bool bIsSuccessful, FName Evaluation)
{
	// Hide UI
	OnQTEStatusChanged.Broadcast(AssignedIndex, false);
	QTEWidgetComponent->SetVisibility(false);

	// broadcast evaluation to boat
	OnFishingResult.Broadcast(Evaluation);

	bLastCatchSuccessful = bIsSuccessful;

	CurrentState = ERodState::Raising;

	if (RodConfig)
	{
		if (RodConfig->FishingReelSound)
		{
			UGameplayStatics::PlaySound2D(this, RodConfig->FishingReelSound);
		}

		if (Evaluation == FName("PERFECT") || Evaluation == FName("FEVER_PERFECT"))
		{
			if (RodConfig->EvalPerfectSound) UGameplayStatics::PlaySound2D(this, RodConfig->EvalPerfectSound);
		}
		else if (Evaluation == FName("GOOD"))
		{
			if (RodConfig->EvalGoodSound) UGameplayStatics::PlaySound2D(this, RodConfig->EvalGoodSound);
		}
		else if (Evaluation.ToString().Contains(TEXT("MISS")) || Evaluation == FName("PUNISH"))
		{
			if (RodConfig->EvalMissSound) UGameplayStatics::PlaySound2D(this, RodConfig->EvalMissSound);
		}
	}
}

//void AFishingRodActor::OnFishEscape()
//{
//	if (CurrentState == ERodState::FishOnHook)
//	{
//		CurrentState = ERodState::WaitingForBite;
//		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Fish Escape"));
//
//		float RandomWaitTime = FMath::FRandRange(3.0f, 6.0f);
//		GetWorldTimerManager().SetTimer(BiteTimerHandle, this, &AFishingRodActor::OnFishBite, RandomWaitTime, false);
//	}
//
//}

void AFishingRodActor::TryInteract()
{
	// If no fish bite hook
	if (CurrentState == ERodState::Idle ||
		CurrentState == ERodState::Lowering ||
		CurrentState == ERodState::WaitingForBite)
	{
		// Clear timer
		GetWorldTimerManager().ClearTimer(BiteTimerHandle);

		TriggerMoveToRaising(false, FName("EMPTY_RETRACT"));

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White, TEXT("The fishing rod was forcibly retracted and reset without any conditions..."));
		return;
	}

	if (CurrentState == ERodState::FishOnHook)
	{
		EvaluateQTEStrike();
	}
}

void AFishingRodActor::EvaluateQTEStrike()
{
	// Get cuurent time progess
	float RemainingTime = GetWorldTimerManager().GetTimerRemaining(QTEMissTimerHandle);
	float ElapsedTime = QTEWindow - RemainingTime;
	float ProgressRatio = FMath::Clamp(ElapsedTime / QTEWindow, 0.0f, 1.0f);

	// Clear Time
	GetWorldTimerManager().ClearTimer(QTEMissTimerHandle);

	if (bIsFeverActive)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Magenta, TEXT("FEVER!!!"));
		TriggerMoveToRaising(true, FName("FEVER_PERFECT"));

		return;
	}

	// get fish data
	FFishSpeciesData* FishData = CurrentLockedFishRow.GetRow<FFishSpeciesData>(TEXT("Context_Strike_Check"));

	float CStart = 0.3f; float CEnd = 0.7f;
	float PStart = 0.48f; float PEnd = 0.52f;

	if (FishData)
	{
		CStart = FishData->CatchStartRatio;
		CEnd = FishData->CatchEndRatio;
		PStart = FishData->PerfectStartRatio;
		PEnd = FishData->PerfectEndRatio;
	}

	if (ProgressRatio < CStart)
	{
		// too early
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Miss"));
		TriggerMoveToRaising(false, FName("EARLY_MISS"));
	}
	else if (ProgressRatio >= PStart && ProgressRatio <= PEnd)
	{
		// perfect
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, TEXT("Perfect"));
		TriggerMoveToRaising(true, FName("PERFECT"));
	}
	else if ((ProgressRatio >= CStart && ProgressRatio < PStart) || (ProgressRatio > PEnd && ProgressRatio <= CEnd))
	{
		// good
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Good"));
		TriggerMoveToRaising(true, FName("GOOD"));
	}
	else
	{
		// late
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Miss"));
		TriggerMoveToRaising(false, FName("LATE_MISS"));
		//TriggerResetToWaiting(FName("LATE_MISS"));
	}
}

// Called every frame
void AFishingRodActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (CurrentState == ERodState::Lowering)
	//{
	//	Lowering(DeltaTime);
	//}
	//else if (CurrentState == ERodState::Raising)
	//{
	//	Raising(DeltaTime);
	//}
	//else if (CurrentState == ERodState::FishOnHook)
	//{
	//	float Shake = FMath::Sin(GetWorld()->GetTimeSeconds() * 40.0f) * 2.0f;
	//	RodMesh->SetRelativeRotation(FRotator(-45.0f + Shake, 0.f, 0.f));
	//}

	//float CurrentRaiseSpeed = RodConfig ? RodConfig->RaiseSpeed : 5.0f;
	//float CurrentLowerSpeed = RodConfig ? RodConfig->LowerSpeed : 1.0f;
	//float MaxAngle = RodConfig ? RodConfig->RodMaxAngle : 45.0f;

	if (CurrentState == ERodState::Idle)
	{
		if (VisualProgress < 1.0f)
		{
			VisualProgress += DeltaTime * RodConfig->RaiseSpeed;

			if (VisualProgress >= 1.0f)
			{
				VisualProgress = 1.0f;
			}

			RodMesh->SetRelativeRotation(FRotator(-RodConfig->RodMaxAngle * (1.0f - VisualProgress), 0.f, 0.f));
		}
	}
	else if (CurrentState == ERodState::Lowering)
	{
		VisualProgress -= DeltaTime * RodConfig->LowerSpeed;
		RodMesh->SetRelativeRotation(FRotator(-RodConfig->RodMaxAngle * (1.0f - VisualProgress), 0.f, 0.f));

		if (VisualProgress <= 0.0f)
		{
			VisualProgress = 0.0f;
			CurrentState = ERodState::WaitingForBite;

			//float FinalWaitMin = RodConfig->BiteWaitTimeMin * CurrentEnvWaitMin;
			//float FinalWaitMax = RodConfig->BiteWaitTimeMax * CurrentEnvWaitMax;

			//float RandomBiteTime = FMath::FRandRange(FinalWaitMin, FinalWaitMax);

			float RandomBiteTime = 0.0f;

			if (bIsFeverActive)
			{
				RandomBiteTime = FMath::FRandRange(RodConfig->FeverBiteTime.Min, RodConfig->FeverBiteTime.Max);
			}
			else
			{
				float FinalWaitMin = RodConfig->BiteWaitTimeMin * CurrentEnvWaitMin;
				float FinalWaitMax = RodConfig->BiteWaitTimeMax * CurrentEnvWaitMax;
				RandomBiteTime = FMath::FRandRange(FinalWaitMin, FinalWaitMax);
			}

			GetWorldTimerManager().SetTimer(BiteTimerHandle, this, &AFishingRodActor::OnFishBite, RandomBiteTime, false);

			// set bite hook time
			//GetWorldTimerManager().SetTimer(BiteTimerHandle, this, &AFishingRodActor::OnFishBite, FMath::FRandRange(RodConfig->BiteWaitTimeMin, RodConfig->BiteWaitTimeMax), false);
		}
	}
	else if (CurrentState == ERodState::Raising)
	{
		VisualProgress += DeltaTime * RodConfig->RaiseSpeed;
		RodMesh->SetRelativeRotation(FRotator(-RodConfig->RodMaxAngle * (1.0f - VisualProgress), 0.f, 0.f));

		if (VisualProgress >= 1.0f)
		{
			VisualProgress = 1.0f;
			//StartLowering();
			CurrentState = ERodState::Idle;

			if (bLastCatchSuccessful && !CurrentLockedFishRow.IsNull())
			{
				FVector SpawnLoc = RodMesh->GetComponentLocation() + RodMesh->GetUpVector() * 300.0f;
				OnFishCaughtSuccess.Broadcast(CurrentLockedFishRow, SpawnLoc);
			}

			// clear old data
			CurrentLockedFishRow.DataTable = nullptr;
			CurrentLockedFishRow.RowName = NAME_None;
			bLastCatchSuccessful = false;

			GetWorldTimerManager().SetTimer(AutoRecastTimerHandle, this, &AFishingRodActor::StartLowering, RodConfig->ResetWaitTime, false);
		}
	}
	else if (CurrentState == ERodState::FishOnHook)
	{
		QTEProgress += DeltaTime / QTEWindow;

		// shake
		float Shake = FMath::Sin(GetWorld()->GetTimeSeconds() * RodConfig->ShakeFrequency) * RodConfig->ShakeAmplitude;
		RodMesh->SetRelativeRotation(FRotator(-RodConfig->RodMaxAngle + Shake, 0.f, 0.f));
	}
}

