// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FishingBoatConfig.generated.h"

class ABombActor;
class UNiagaraSystem;
class USoundBase;

/**
 * 
 */
UCLASS()
class FISHINGMASTER_API UFishingBoatConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- Movement & Handling ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float MaxForwardSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float MaxReverseSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float ForwardAcceleration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float ForwardDeceleration = 1.0f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float MaxTurnSpeed = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float TurnAcceleration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float MaxRollAngle = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float PropellerRotationSpeed = 720.0f;

	// --- Camera ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraInterpSpeed = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float DrivingCameraLength = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float FishingCameraLength = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float DrivingDefaultPitch = -20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float FishingPitch = -35.0f;

	// --- Gameplay ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	int32 FeverComboThreshold = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	float FeverDuration = 10.0f;

	// Idle sea area guarantee system
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fallback Fishing")
	TArray<FDataTableRowHandle> FallbackTrashPool;

	// The probability of triggering a trash catch (for example, 0.4 means there is a 40% chance of catching trash, while there is a 60% chance of nothing happening).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fallback Fishing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TrashCatchProbability = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	TSubclassOf<ABombActor> BombClass;
	//TObjectPtr<ABombActor> BombClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> BoatEngineLoopSound;
};

UCLASS()
class FISHINGMASTER_API UBombConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	//TSubclassOf<ABombActor> BombClass;

		// Time before explosion
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	float ExplosionDelay = 2.0f;

	// Radius of explosion
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	float ExplosionRadius = 800.0f;

	// 	// air damping
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water Physics")
	float AirDamping = 0.01f;

	// The higher the value, the more violently the deceleration occurs after falling into the water.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Water Physics")
	float WaterDamping = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> BoomVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> WaterSplashVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> BombSplashSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> BombExplosionSound;
};