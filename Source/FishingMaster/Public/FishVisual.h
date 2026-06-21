// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishVisual.generated.h"

class UBoxComponent;
class AOceanManager;

UCLASS()
class FISHINGMASTER_API AFishVisual : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFishVisual();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* FishMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* SkeletalFishMesh;

	UFUNCTION(BlueprintCallable, Category = "Visual")
	virtual void PlayDespawnEffect();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

UCLASS()
class FISHINGMASTER_API AFishCaughtVisual : public AFishVisual
{
	GENERATED_BODY()
public:

	AFishCaughtVisual();

	UFUNCTION(BlueprintImplementableEvent, Category = "Flight")
	void StartFlight(FVector TargetLocation);
};

UCLASS()
class FISHINGMASTER_API AFishBombedVisual : public AFishVisual
{
	GENERATED_BODY()
public:
	AFishBombedVisual();

	void BeginPlay();

	virtual void Tick(float DeltaTime) override;

	// 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> PickupCollision;

	UPROPERTY(BlueprintReadOnly, Category = "Fish Data")
	FDataTableRowHandle FishDataHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Float")
	float FloatUpSpeed = 300.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Float")
	bool bIsOnSurface = false;


	float RandomBobbingPhase;

	TObjectPtr< AOceanManager> OceanManager;

	//UPROPERTY()
	//TObjectPtr< AActor> PushingActor = nullptr;


	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Float")
	//float RepulsionRadius = 400.0f;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Float")
	//float RepulsionForce = 200.0f;

	//void SetPushedBy(AActor* InActor);

	//UFUNCTION(BlueprintImplementableEvent, Category = "Float")
	//void StartFloatingUp();
};
