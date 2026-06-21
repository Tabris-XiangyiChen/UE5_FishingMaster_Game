// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_ThirdPersonGameMode.h"
#include "TP_ThirdPersonCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "FishingBoat.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

ATP_ThirdPersonGameMode::ATP_ThirdPersonGameMode()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	//if (PlayerPawnBPClass.Class != NULL)
	//{
	//	DefaultPawnClass = PlayerPawnBPClass.Class;
	//}
	DefaultPawnClass = nullptr;
}

void ATP_ThirdPersonGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (!BoatClassToSpawn)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: No BoatClassToSpawn"));
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    AActor* StartSpot = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass());

    FTransform SpawnTransform = StartSpot ? StartSpot->GetActorTransform() : FTransform(FVector(0.f, 0.f, 200.f));

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AFishingBoat* SpawnedBoat = GetWorld()->SpawnActor<AFishingBoat>(BoatClassToSpawn, SpawnTransform, SpawnParams);

    if (SpawnedBoat)
    {
        PC->Possess(SpawnedBoat);
        UE_LOG(LogTemp, Warning, TEXT("GameMode: Spawn Player Ship succeed£¡"));
    }
}
