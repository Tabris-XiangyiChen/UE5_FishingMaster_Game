// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmbientFishSwarm.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class AOceanManager;

UCLASS()
class FISHINGMASTER_API AAmbientFishSwarm : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAmbientFishSwarm();

	// Fish particle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UNiagaraSystem> AmbientVFX;

	// Partical move speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RoamSpeed = 150.0f;

	// Monv radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RoamRadius = 2000.0f;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> SwarmVFX;

	UPROPERTY()
	TObjectPtr<AOceanManager> OceanManager;
private:
	FVector StartLocation;
	FVector TargetLocation;

	void PickNewTargetLocation();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
