// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Config/FishSwarmConfig.h"

#include "FishSwarmActor.generated.h"

class UCapsuleComponent;
class UNiagaraComponent;
class AOceanManager;

UCLASS()
class FISHINGMASTER_API AFishSwarmActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFishSwarmActor();

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	FDataTableRowHandle DrawFishFromPool();

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void OnBombed(FVector BombLocation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm Settings")
	TSubclassOf<class AFishBombedVisual> FloatingFishClass;

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	bool HasFishesLeft() const { return CurrentCapacity > 0; }

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void InitializeSwarm(UFishSwarmConfig* AssignedConfig);

	TObjectPtr<UFishSwarmConfig> GetCurrentConfig() const { return CurrentConfig; };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<AOceanManager> OceanManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> SwarmZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> SwarmVFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ValueIndicatorComp;

	// // Random fish swarm pool
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swarm Settings")
	// TArray<TObjectPtr<UFishSwarmConfig>> PossibleConfigs;

	// Current config state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Swarm Status")
	TObjectPtr<UFishSwarmConfig> CurrentConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Swarm Status")
	int32 CurrentCapacity = 0;

	// Indicator State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Swarm Status")
	bool bIsIndicatorActive = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Swarm Status")
	float IndicatorBaseZ = 0.0f;

	UFUNCTION()
	void OnSwarmOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSwarmOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

// private:
// 	void DetermineSwarmType();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
