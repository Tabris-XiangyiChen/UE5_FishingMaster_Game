// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TP_ThirdPersonGameMode.generated.h"

class AFishingBoat;

UCLASS(minimalapi)
class ATP_ThirdPersonGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATP_ThirdPersonGameMode();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    TSubclassOf<AFishingBoat> BoatClassToSpawn;
};



