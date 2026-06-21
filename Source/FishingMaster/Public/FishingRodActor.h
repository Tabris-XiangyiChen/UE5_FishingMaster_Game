// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishingRodActor.generated.h"

// Rod State
UENUM(BlueprintType)
enum class ERodState : uint8
{
	Idle,
	Lowering,
	WaitingForBite,
	FishOnHook,
	Raising
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQTEStatusChanged, int32, RodIndex, bool, bActivate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishingResult, FName, ResultText); // "PERFECT", "MISS"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRodRequestFishData, int32, RodIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFishCaughtSuccess, FDataTableRowHandle, CaughtFishRow, FVector, SpawnLocation);

class UFishingRodConfig;
class UWidgetComponent;

UCLASS()
class FISHINGMASTER_API AFishingRodActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFishingRodActor();

	void ActivateRod(int32 InRodIndex);
	void DeactivateRod(); 
	void Interact();

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void TryInteract();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnQTEStatusChanged OnQTEStatusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFishingResult OnFishingResult;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFishCaughtSuccess OnFishCaughtSuccess;

	UFUNCTION()
	void SetFeverState(bool bFever);

	UFUNCTION(BlueprintImplementableEvent, Category = "QTE UI")
	void ToggleFeverUI(bool bIsFever);

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void SetEnvironmentModifiers(float MinMultiplier, float MaxMultiplier);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRodRequestFishData OnRodRequestFishData;

	// After the the boat draws the card, it forcibly feeds the fish's data into the fishing rod.
	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void AssignFishData(FDataTableRowHandle FishRowHandle);

	UFUNCTION()
	void TriggerResetToWaiting(FName Evaluation);

	// UI
	// QTE UI 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* QTEWidgetComponent;

	// Get QTE progess (0.0 ~ 1.0)
	UFUNCTION(BlueprintCallable, Category = "QTE UI")
	float GetCurrentQTEProgress() const { return QTEProgress; }

	// Init qte ui
	UFUNCTION(BlueprintImplementableEvent, Category = "QTE UI")
	void InitQTEUI(float CatchStart, float CatchEnd, float PerfectStart, float PerfectEnd);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* RodMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UFishingRodConfig> RodConfig;


	bool bIsFeverActive = false;
	bool bLastCatchSuccessful = false;

private:
	//void Lowering(float Time);

	//void Raising(float Time);

	void StartLowering();
	void OnFishBite();
	void TriggerMoveToRaising(bool bIsSuccessful, FName Evaluation);

	void EvaluateQTEStrike();

	ERodState CurrentState = ERodState::Idle;
	int32 AssignedIndex = 0;

	float QTEProgress = 0.0f;
	float QTEWindow = 2.0f;

	float VisualProgress = 0.0f;
	//float RaiseSpeed = 5.0f;
	//float LowerSpeed = 1.0f;

	FTimerHandle BiteTimerHandle;
	FTimerHandle QTEMissTimerHandle;
	FTimerHandle AutoRecastTimerHandle;

	// The fishing rod automatically saves its environmental coefficient, which is 1.0 by default (normal time).
	float CurrentEnvWaitMin = 1.0f;
	float CurrentEnvWaitMax = 1.0f;

	// The global table handle that records the fish currently biting this rod.
	FDataTableRowHandle CurrentLockedFishRow;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
