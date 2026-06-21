// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BombActor.generated.h"

class UStaticMeshComponent;
class AOceanManager;
class UBombConfig;

UCLASS()
class FISHINGMASTER_API ABombActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABombActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BombMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UBombConfig> BombConfig;

	// check if has in water
	UPROPERTY(BlueprintReadOnly, Category = "Water Physics")
	bool bHasEnteredWater = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "VFX")
	void PlayWaterSplashVFX(FVector SplashLocation);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Explode();

	UPROPERTY()
	TObjectPtr<AOceanManager> OceanManager;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
