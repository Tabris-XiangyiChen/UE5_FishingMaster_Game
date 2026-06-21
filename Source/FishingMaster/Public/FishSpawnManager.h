// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishSpawnManager.generated.h"

class UBoxComponent;
class AFishSwarmActor;
class AAmbientFishSwarm;
class AOceanManager;
class UFishManagerConfig;


UCLASS()
class FISHINGMASTER_API AFishSpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFishSpawnManager();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> SpawnArea;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> LimitationPlane;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boundary")
    TObjectPtr<UBoxComponent> NorthWall;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boundary")
    TObjectPtr<UBoxComponent> SouthWall;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boundary")
    TObjectPtr<UBoxComponent> EastWall;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boundary")
    TObjectPtr<UBoxComponent> WestWall;

    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Settings")
    //TArray<FSwarmSpawnInfo> InteractiveSwarmsToSpawn;

    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Settings")
    //TArray<FSwarmSpawnInfo> AmbientSwarmsToSpawn;

    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Settings")
    //float CheckInterval = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    TObjectPtr<UFishManagerConfig> ManagerConfig;

private:
    void TrySpawnSwarms();
    FTimerHandle SpawnTimerHandle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY()
    TObjectPtr<AOceanManager> OceanManager;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
