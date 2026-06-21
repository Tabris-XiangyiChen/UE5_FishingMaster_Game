// Fill out your copyright notice in the Description page of Project Settings.


#include "BombActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "FishSwarmActor.h"
#include "Engine/OverlapResult.h"
#include "Ocean/OceanManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Config/FishingBoatConfig.h"

// Sets default values
ABombActor::ABombActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
	RootComponent = BombMesh;

	BombMesh->SetSimulatePhysics(true);

    BombMesh->SetNotifyRigidBodyCollision(true);
    BombMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
}

// Called when the game starts or when spawned
void ABombActor::BeginPlay()
{
	Super::BeginPlay();

    OceanManager = Cast<AOceanManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AOceanManager::StaticClass()));

    if (!BombConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("ABombActor: No BombConfig"));

        FTimerHandle ExplodeTimer;
        GetWorldTimerManager().SetTimer(ExplodeTimer, this, &ABombActor::Explode, 2.0f, false);
        return;
    }

    if (BombMesh)
    {
        BombMesh->SetLinearDamping(BombConfig->AirDamping);
        BombMesh->SetAngularDamping(BombConfig->AirDamping);
    }

    FTimerHandle ExplodeTimer;
    GetWorldTimerManager().SetTimer(ExplodeTimer, this, &ABombActor::Explode, BombConfig->ExplosionDelay, false);
}

void ABombActor::Explode()
{
    FVector ExplodeLoc = GetActorLocation();

    // VFX/SFX 
    if (BombConfig)
    {
        if (BombConfig->BoomVFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BombConfig->BoomVFX, ExplodeLoc);
        }

        if (BombConfig->BombExplosionSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, BombConfig->BombExplosionSound, ExplodeLoc);
        }
    }

    // Scan for Fish Swarms
    float CurrentExplosionRadius = BombConfig ? BombConfig->ExplosionRadius : 800.0f;

    TArray<FOverlapResult> OverlapResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(CurrentExplosionRadius);

    GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        ExplodeLoc,
        FQuat::Identity,
        ECC_WorldDynamic, // Ensure FishSwarm capsule uses this channel
        Sphere
    );

    // Trigger bombed event
    for (const FOverlapResult& Result : OverlapResults)
    {
        AFishSwarmActor* Swarm = Cast<AFishSwarmActor>(Result.GetActor());
        if (Swarm)
        {
            Swarm->OnBombed(ExplodeLoc);
        }
    }

    Destroy();
}

// Called every frame
void ABombActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (OceanManager && !bHasEnteredWater)
    {
        FVector CurrentLoc = GetActorLocation();
        float WaterZ = OceanManager->GetApproximateOceanHeight(CurrentLoc);

        // Check Z
        if (CurrentLoc.Z <= WaterZ)
        {
            bHasEnteredWater = true;
            FVector SplashLoc = FVector(CurrentLoc.X, CurrentLoc.Y, WaterZ);

            if (BombConfig)
            {
                // Changing the physical state, increasing damping to simulate water resistance.
                if (BombMesh)
                {
                    BombMesh->SetLinearDamping(BombConfig->WaterDamping);
                    // The angle damping is slightly smaller, allowing the bomb to still tumble slightly in the water.
                    BombMesh->SetAngularDamping(BombConfig->WaterDamping * 0.5f);

                }

                if (BombConfig->WaterSplashVFX)
                {
                    // Generate niagara particle
                    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                        GetWorld(),
                        BombConfig->WaterSplashVFX,
                        SplashLoc,
                        FRotator::ZeroRotator 
                    );
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("WaterSplashVFX is missing in Bomb Blueprint!"));
                }

                if (BombConfig->BombSplashSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(this, BombConfig->BombSplashSound, SplashLoc);
                }
            }
            PlayWaterSplashVFX(SplashLoc);


            UE_LOG(LogTemp, Warning, TEXT("Bomb entered water at Z: %f"), WaterZ);
        }
    }
}

