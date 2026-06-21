#include "FishingBoat.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
//#include "Components/SceneComponent"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FishingRodActor.h"
#include "Config/FishingBoatConfig.h"
#include "DrawDebugHelpers.h"
#include "Config/FishSwarmConfig.h"
//#include "FishVisual.h"
#include "FishSwarmActor.h"
#include "Ocean/OceanManager.h"
#include "Ocean/OceanConfig.h"
#include "Kismet/GameplayStatics.h"
#include "BombActor.h"
#include "UI/FishingHUD.h"
#include "FishingPlayerController.h"
#include "QuestManagerComponent.h"
#include "FishData.h"
#include "Components/AudioComponent.h"

AFishingBoat::AFishingBoat()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root component
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoatCollision"));
    RootComponent = CollisionComponent;
    CollisionComponent->SetBoxExtent(FVector(300.f, 120.f, 100.f));
    CollisionComponent->SetSimulatePhysics(false); 

    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComponent->SetCollisionProfileName(TEXT("Pawn"));

    // Main boat hull
    HullMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HullMesh"));
    HullMesh->SetupAttachment(RootComponent);
    HullMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Child componenets
    PropellerMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropellerMesh1"));
    PropellerMesh1->SetupAttachment(HullMesh); 
    PropellerMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropellerMesh2"));
    PropellerMesh2->SetupAttachment(HullMesh);

    // Fish storage components
    FishStoragePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FishStoragePoint"));
    FishStoragePoint->SetupAttachment(HullMesh);
    FishStoragePoint->SetRelativeLocation(FVector(-300.f, 0.f, 150.f));

    // Fishing Rod components
    RodSlot1 = CreateDefaultSubobject<UChildActorComponent>(TEXT("RodMount1"));
    RodSlot1->SetupAttachment(HullMesh);
    RodSlot2 = CreateDefaultSubobject<UChildActorComponent>(TEXT("RodMount2"));
    RodSlot2->SetupAttachment(HullMesh);
    RodSlot3 = CreateDefaultSubobject<UChildActorComponent>(TEXT("RodMount3"));
    RodSlot3->SetupAttachment(HullMesh);
    RodSlot4 = CreateDefaultSubobject<UChildActorComponent>(TEXT("RodMount4"));
    RodSlot4->SetupAttachment(HullMesh);

    //RodMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RodMesh1"));
    //RodMesh1->SetupAttachment(HullMesh);

    CraneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CraneMesh"));
    CraneMesh->SetupAttachment(HullMesh);
    CraneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CatchFishZone = CreateDefaultSubobject<USphereComponent>(TEXT("CatchFishZone"));
    // CatchFishZone->SetupAttachment(CraneMesh, TEXT("NetSocket")); 
    CatchFishZone->SetupAttachment(CraneMesh);
    CatchFishZone->SetSphereRadius(150.f);
    CatchFishZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CatchFishZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    CatchFishZone->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    // CatchFishZone->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);

    // Camera
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 1200.f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bDoCollisionTest = false;

    ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
    ViewCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

    EngineAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
    EngineAudioComp->SetupAttachment(RootComponent);
}

void AFishingBoat::BeginPlay()
{
    Super::BeginPlay();

    if (!BoatConfig)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("NO Boat config dataasset !"));
        SetActorTickEnabled(false);
        return;
    }

    if (!OceanManager)
    {
        OceanManager = Cast<AOceanManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AOceanManager::StaticClass()));

        if (!OceanManager)
        {
            UE_LOG(LogTemp, Error, TEXT("FishingBoat: ³¡¾°ÖÐÃ»ÓÐ·ÅÖÃ OceanManager£¡"));
        }
    }

    // IMC_Default
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DrivingMappingContext, 0);
        }

        FRotator InitialRot = PlayerController->GetControlRotation();
        InitialRot.Pitch = BoatConfig->DrivingDefaultPitch;
        PlayerController->SetControlRotation(InitialRot);
    }

    // load rods
    auto AddRodToArray = [this](TObjectPtr<UChildActorComponent> Mount)
        {
            if (Mount && Mount->GetChildActor())
            {
                if (AFishingRodActor* RealRod = Cast<AFishingRodActor>(Mount->GetChildActor()))
                {
                    AllFishingRods.Add(RealRod);

                    OnFeverStateChanged.AddDynamic(RealRod, &AFishingRodActor::SetFeverState);

                    RealRod->OnFishingResult.AddDynamic(this, &AFishingBoat::ReportFishingScore);

                    RealRod->OnRodRequestFishData.AddDynamic(this, &AFishingBoat::HandleRodRequestFishData);

                    RealRod->OnFishCaughtSuccess.AddDynamic(this, &AFishingBoat::HandleFishCaughtSuccess);
                }
            }
        };

    AddRodToArray(RodSlot1);
    AddRodToArray(RodSlot2);
    AddRodToArray(RodSlot3);
    AddRodToArray(RodSlot4);

    if (CollisionComponent)
    {
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AFishingBoat::OnBoatOverlapBegin);
        CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AFishingBoat::OnBoatOverlapEnd);
    }

    if (CatchFishZone)
    {
        // Bind overlap fish event
        CatchFishZone->OnComponentBeginOverlap.AddDynamic(this, &AFishingBoat::OnCatchFishZoneOverlap);
    }

    // Init engine audio
    if (!bIsMainMenuProp)
    {
        if (EngineAudioComp && BoatConfig && BoatConfig->BoatEngineLoopSound)
        {
            EngineAudioComp->SetSound(BoatConfig->BoatEngineLoopSound);
            EngineAudioComp->Play();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Success install rod num: %d"), AllFishingRods.Num());
}

void AFishingBoat::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (PropellerMesh1)
    {
        PropellerMesh1->AddLocalRotation(FRotator(0.f, 0.f, BoatConfig->PropellerRotationSpeed * DeltaTime * ForwardInput));
    }
    if (PropellerMesh2)
    {
        PropellerMesh2->AddLocalRotation(FRotator(0.f, 0.f, BoatConfig->PropellerRotationSpeed * DeltaTime * ForwardInput));
    }

    // Move logic
    float CurrentMaxSpeed = (ForwardInput >= 0.0f) ? BoatConfig->MaxForwardSpeed : BoatConfig->MaxReverseSpeed;

    float TargetForwardSpeed = ForwardInput * CurrentMaxSpeed;

    float CurrentForwardAccel = (FMath::Abs(ForwardInput) > 0.1f) ? BoatConfig->ForwardAcceleration : BoatConfig->ForwardDeceleration;

    CurrentForwardSpeed = FMath::FInterpTo(CurrentForwardSpeed, TargetForwardSpeed, DeltaTime, CurrentForwardAccel);

    if (FMath::Abs(CurrentForwardSpeed) > 1.0f)
    {
        FVector DeltaLocation = FVector(CurrentForwardSpeed * DeltaTime, 0.f, 0.f);
        AddActorLocalOffset(DeltaLocation, true);
        //AddActorLocalOffset(DeltaLocation, false);
    }

    // back forward
    float SteerMultiplier = 1.0f;
    if (ForwardInput < 0.0f || (FMath::IsNearlyZero(ForwardInput) && CurrentForwardSpeed < -1.0f))
    {
        SteerMultiplier = -1.0f;
    }

    float TargetTurnSpeed = TurnInput * SteerMultiplier * BoatConfig->MaxTurnSpeed;

    CurrentTurnSpeed = FMath::FInterpTo(CurrentTurnSpeed, TargetTurnSpeed, DeltaTime, BoatConfig->TurnAcceleration);

    if (FMath::Abs(CurrentTurnSpeed) > 0.1f)
    {
        FRotator DeltaRotation = FRotator(0.f, CurrentTurnSpeed * DeltaTime, 0.f);
        //AddActorLocalRotation(DeltaRotation, true);
        AddActorLocalRotation(DeltaRotation, false);

        // Boat tilting effect
        float TargetRoll = -(TurnInput  * SteerMultiplier) * BoatConfig->MaxRollAngle;
        FRotator CurrentRotation = HullMesh->GetRelativeRotation();
        float SmoothedRoll = FMath::FInterpTo(CurrentRotation.Roll, TargetRoll, DeltaTime, 2.0f);
        HullMesh->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, SmoothedRoll));
    }
    else
    {
        // Back to normal
        FRotator CurrentRotation = HullMesh->GetRelativeRotation();
        float SmoothedRoll = FMath::FInterpTo(CurrentRotation.Roll, 0.0f, DeltaTime, 2.0f);
        HullMesh->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, SmoothedRoll));
    }

    // Camera control
    float TargetLength = bIsFishingMode ? BoatConfig->FishingCameraLength : BoatConfig->DrivingCameraLength;
    if (!FMath::IsNearlyEqual(SpringArm->TargetArmLength, TargetLength, 0.5f))
    {
        SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetLength, DeltaTime, BoatConfig->CameraInterpSpeed);
    }
    else
    {
        SpringArm->TargetArmLength = TargetLength;
    }

    if (bIsFishingMode)
    {
        if (APlayerController* PC = Cast<APlayerController>(Controller))
        {
            FRotator TargetRot = FRotator(BoatConfig->FishingPitch, GetActorRotation().Yaw, 0.0f);
            FRotator CurrentRot = PC->GetControlRotation();

            // Use the `Equals` function to check. If the error between the current camera angle 
            // and the target angle is less than 0.05 degrees, it means the target is fully in position.
            //if (!SpringArm->GetComponentRotation().Equals(TargetRot, 0.05f))
            //{
            //    FRotator NewRot = FMath::RInterpTo(SpringArm->GetComponentRotation(), TargetRot, DeltaTime, BoatConfig->CameraInterpSpeed);
            //    SpringArm->SetWorldRotation(NewRot);
            //}
            //else
            //{
            //    SpringArm->SetWorldRotation(TargetRot);
            //}
            if (!CurrentRot.Equals(TargetRot, 0.05f))
            {
                FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, BoatConfig->CameraInterpSpeed);
                PC->SetControlRotation(NewRot);
            }
            else
            {
                PC->SetControlRotation(TargetRot);
            }
        }
    }

    // Draw debug text
    if (CurrentSwarm != nullptr)
    {
        FVector TextLocation = GetActorLocation() + FVector(0.0f, 0.0f, 300.0f);

        //DrawDebugString(GetWorld(), TextLocation, TEXT("|| IN FISH AREA ||"), nullptr, FColor::Green, 0.0f, true, 2.0f);
    }

    if (EngineAudioComp && BoatConfig && BoatConfig->BoatEngineLoopSound)
    {
        float SpeedRatio = FMath::Clamp(FMath::Abs(CurrentForwardSpeed) / BoatConfig->MaxForwardSpeed, 0.0f, 1.0f);
        EngineAudioComp->SetPitchMultiplier(FMath::Lerp(0.8f, 1.5f, SpeedRatio));
        EngineAudioComp->SetVolumeMultiplier(FMath::Lerp(0.5f, 1.0f, SpeedRatio));
    }

    CalculateBuoyancy(DeltaTime);
}

void AFishingBoat::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFishingBoat::EnhancedMove);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AFishingBoat::EnhancedMoveStop);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AFishingBoat::EnhancedMoveStop);

        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFishingBoat::EnhancedLook);
        EnhancedInputComponent->BindAction(BombAction, ETriggerEvent::Started, this, &AFishingBoat::DropBomb);

        EnhancedInputComponent->BindAction(ToggleFishingModeAction, ETriggerEvent::Started, this, &AFishingBoat::ToggleFishingMode);

        EnhancedInputComponent->BindAction(TriggerRod1Action, ETriggerEvent::Started, this, &AFishingBoat::TriggerRod1);
        EnhancedInputComponent->BindAction(TriggerRod2Action, ETriggerEvent::Started, this, &AFishingBoat::TriggerRod2);
        EnhancedInputComponent->BindAction(TriggerRod3Action, ETriggerEvent::Started, this, &AFishingBoat::TriggerRod3);
        EnhancedInputComponent->BindAction(TriggerRod4Action, ETriggerEvent::Started, this, &AFishingBoat::TriggerRod4);
    }
}

void AFishingBoat::OnBoatOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (AFishSwarmActor* Swarm = Cast<AFishSwarmActor>(OtherActor))
    {
        EnterSwarm(Swarm);
    }
}

void AFishingBoat::OnBoatOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (AFishSwarmActor* Swarm = Cast<AFishSwarmActor>(OtherActor))
    {
        LeaveSwarm(Swarm);
    }
}

void AFishingBoat::OnCatchFishZoneOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (AFishBombedVisual* BombedFish = Cast<AFishBombedVisual>(OtherActor))
    {
        UE_LOG(LogTemp, Error, TEXT("Eat Fish！"));

        // get fish data
        FDataTableRowHandle FishIdentity = BombedFish->FishDataHandle;

        if (!FishIdentity.IsNull())
        {
            FFishSpeciesData* FishData = FishIdentity.GetRow<FFishSpeciesData>(TEXT("Harvest Lookup"));
            if (FishData)
            {
                UE_LOG(LogTemp, Warning, TEXT("Boat: Successfully harvested bombed fish: %s"), *FishData->FishName.ToString());

                // Play Collect Sound
                if (FishData->FishCollectSound)
                {
                    UGameplayStatics::PlaySound2D(this, FishData->FishCollectSound);
                }

                // Quest Manager 
                if (APlayerController* PC = Cast<APlayerController>(GetController()))
                {
                    if (AFishingPlayerController* FPC = Cast<AFishingPlayerController>(PC))
                    {
                        if (FPC->QuestManager)
                        {
                            FPC->QuestManager->ReportAction(FishIdentity.RowName, 1, true);
                        }
                    }

                    // HUD Loot Notification
                    if (AFishingHUD* HUD = Cast<AFishingHUD>(PC->GetHUD()))
                    {
                        HUD->ShowLootNotification(FishData->FishName, 1, FishData->FishIcon);
                    }
                }
            }
        }

        BombedFish->Destroy();
    }
}

void AFishingBoat::HandleFishCaughtSuccess(FDataTableRowHandle CaughtFishRow, FVector SpawnLocation)
{
    FFishSpeciesData* FishData = CaughtFishRow.GetRow<FFishSpeciesData>(TEXT("Spawn_Caught_Fish"));

    if (FishData && FishData->CaughtFishClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // Spwan fishi
        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(FishData->CaughtFishClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

        if (AFishCaughtVisual* VisualFish = Cast<AFishCaughtVisual>(SpawnedActor))
        {
            // fly to storage
            FVector TargetLoc = FishStoragePoint->GetComponentLocation();
            VisualFish->StartFlight(TargetLoc);
        }

        // Trigger  UI Notification via HUD
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            if (AFishingPlayerController* FPC = Cast<AFishingPlayerController>(PC))
            {
                if (FPC->QuestManager)
                {
                    FPC->QuestManager->ReportAction(CaughtFishRow.RowName, 1, false);
                }
            }

            if (AFishingHUD* HUD = Cast<AFishingHUD>(PC->GetHUD()))
            {
                UE_LOG(LogTemp, Warning, TEXT("Boat: Catch success. Sending [%s] to HUD."), *FishData->FishName.ToString());

                // Push Loot Notification
                HUD->ShowLootNotification(FishData->FishName, 1, FishData->FishIcon);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Boat: HUD or PC is null. Cannot push UI."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Boat: HUD or PC is null. Cannot push UI."));
        }

        // Play Collect Sound
        if (FishData->FishCollectSound)
        {
            UGameplayStatics::PlaySound2D(this, FishData->FishCollectSound);
        }
    }
}

//float AFishingBoat::GetApproximateOceanHeight(FVector WorldLocation)
//{
//    // Check if ocean exists. If not, return default water level
//    if (!OceanManager || !OceanManager->OceanConfig || OceanManager->OceanConfig->CascadePresets.Num() == 0)
//    {
//        return WaterLineOffset;
//    }
//
//    // Get wind direction and convert UE units (cm) to meters
//    FVector2D WindDir = OceanManager->OceanConfig->WindDirection.GetSafeNormal();
//    FVector2D Pos_m(WorldLocation.X * 0.01f, WorldLocation.Y * 0.01f);
//
//    float BaseTime = GetWorld()->GetTimeSeconds();
//    float TotalHeight_m = 0.0f;
//    float CPUHeightScalar = 0.25f; // Scale down CPU wave height to match GPU visual
//
//    int32 Cascades = OceanManager->OceanConfig->CascadePresets.Num();
//
//    // Loop through wave layers. Start from 1 to skip the biggest wave (Cascade 0)
//    for (int32 i = 1; i < FMath::Min(3, Cascades); i++)
//    {
//        float PatchLength = OceanManager->OceanConfig->CascadePresets[i].PatchLength;
//        float EnergyAmplitude = OceanManager->OceanConfig->CascadePresets[i].WaveAmplitude;
//        float CurrentTimeScale = OceanManager->OceanConfig->CascadePresets[i].TimeScale;
//
//        // Only calculate if wave length is valid
//        if (PatchLength > 0.01f)
//        {
//            // Math for the main wave (k = wave number, omega = frequency)
//            float k1 = (2.0f * PI) / PatchLength;
//            float omega1 = FMath::Sqrt(9.81f * k1);
//
//            FVector2D CurrentWindDir = WindDir;
//            // Rotate wind direction a bit for cascade 1 to look more random
//            if (i == 1)
//            {
//                CurrentWindDir = FVector2D(WindDir.X * 0.9f - WindDir.Y * 0.43f, WindDir.X * 0.43f + WindDir.Y * 0.9f).GetSafeNormal();
//            }
//
//            // Calculate main wave movement over time
//            float phase1 = k1 * FVector2D::DotProduct(CurrentWindDir, Pos_m) - omega1 * (BaseTime * CurrentTimeScale);
//
//            // Create side wave 1 to break the perfect wave shape (destructive interference)
//            FVector2D Dir2(CurrentWindDir.X * 0.866f - CurrentWindDir.Y * 0.5f, CurrentWindDir.X * 0.5f + CurrentWindDir.Y * 0.866f);
//            float k2 = (2.0f * PI) / (PatchLength * 0.8f);
//            float omega2 = FMath::Sqrt(9.81f * k2);
//            float phase2 = k2 * FVector2D::DotProduct(Dir2.GetSafeNormal(), Pos_m) - omega2 * (BaseTime * CurrentTimeScale) + 1.2f;
//
//            // Create side wave 2
//            FVector2D Dir3(CurrentWindDir.X * 0.866f + CurrentWindDir.Y * 0.5f, -CurrentWindDir.X * 0.5f + CurrentWindDir.Y * 0.866f);
//            float k3 = (2.0f * PI) / (PatchLength * 0.9f);
//            float omega3 = FMath::Sqrt(9.81f * k3);
//            float phase3 = k3 * FVector2D::DotProduct(Dir3.GetSafeNormal(), Pos_m) - omega3 * (BaseTime * CurrentTimeScale) + 2.5f;
//
//            // Mix the three waves together (60% main, 20% side1, 20% side2)
//            float RawMathHeight = (0.6f * FMath::Cos(phase1)) + (0.2f * FMath::Cos(phase2)) + (0.2f * FMath::Cos(phase3));
//
//            TotalHeight_m += EnergyAmplitude * RawMathHeight * CPUHeightScalar;
//        }
//    }
//
//    // Convert meters back to UE units (cm) and add base ocean Z position
//    float OceanBaseZ = OceanManager->GetActorLocation().Z;
//    return OceanBaseZ + (TotalHeight_m * 100.0f) + WaterLineOffset;
//}

void AFishingBoat::CalculateBuoyancy(float DeltaTime)
{
    FVector CenterLoc = GetActorLocation();

    // Set 4 check points around the boat (Front, Back, Left, Right)
    FVector ForwardOffset = GetActorForwardVector() * 250.0f;
    FVector RightOffset = GetActorRightVector() * 100.0f;

    FVector FrontLoc = CenterLoc + ForwardOffset;
    FVector BackLoc = CenterLoc - ForwardOffset;
    FVector LeftLoc = CenterLoc - RightOffset;
    FVector RightLoc = CenterLoc + RightOffset;

    // Get water height at the 4 points
    //float FrontZ = GetApproximateOceanHeight(FrontLoc);
    //float BackZ = GetApproximateOceanHeight(BackLoc);
    //float LeftZ = GetApproximateOceanHeight(LeftLoc);
    //float RightZ = GetApproximateOceanHeight(RightLoc);

    float FrontZ = OceanManager->GetApproximateOceanHeight(FrontLoc) + WaterLineOffset;
    float BackZ = OceanManager->GetApproximateOceanHeight(BackLoc) + WaterLineOffset;
    float LeftZ = OceanManager->GetApproximateOceanHeight(LeftLoc) + WaterLineOffset;
    float RightZ = OceanManager->GetApproximateOceanHeight(RightLoc) + WaterLineOffset;

    // Calculate average height for the boat center
    float TargetZ = (FrontZ + BackZ + LeftZ + RightZ) / 4.0f;

    // Calculate target tilt (Pitch and Roll). Multiply to make shaking stronger
    float TargetPitch = FMath::RadiansToDegrees(FMath::Atan2(BackZ - FrontZ, 500.0f)) * PitchMultiplier;
    float TargetRoll = FMath::RadiansToDegrees(FMath::Atan2(LeftZ - RightZ, 200.0f)) * RollMultiplier;

    // Z-Axis Translation (Actor Level)
    FVector CurrentActorLoc = GetActorLocation();

    // Move the whole boat up and down smoothly. Keep speed high to stick to water
    float SmoothedZ = FMath::FInterpTo(CurrentActorLoc.Z, TargetZ, DeltaTime, BuoyancyDamping);
    SetActorLocation(FVector(CurrentActorLoc.X, CurrentActorLoc.Y, SmoothedZ));

    // Decoupled Rotation Physics (HullMesh Level)
    // Wave rotation. Fast speed to follow water waves
    float WaveRotSpeed = BuoyancyDamping * 0.8f;
    FRotator TargetWaveRot = FRotator(TargetPitch, HullMesh->GetRelativeRotation().Yaw, TargetRoll);

    // Smoothly update wave tilt
    CurrentWaveRotation = FMath::RInterpTo(CurrentWaveRotation, TargetWaveRot, DeltaTime, WaveRotSpeed);

    // Turning tilt (centrifugal force). Slower speed for heavy boat feel
    float SteerMultiplier = 1.0f;
    // Reverse tilt direction if the boat is moving backward
    if (ForwardInput < 0.0f || (FMath::IsNearlyZero(ForwardInput) && CurrentForwardSpeed < -1.0f))
    {
        SteerMultiplier = -1.0f;
    }
    float TargetTurnRoll = -(TurnInput * SteerMultiplier) * BoatConfig->MaxRollAngle;

    // Smoothly update turning tilt
    CurrentSteerRoll = FMath::FInterpTo(CurrentSteerRoll, TargetTurnRoll, DeltaTime, TurnRotateSpeed);

    // Combine wave tilt and turning tilt
    FRotator FinalRotation = CurrentWaveRotation;
    FinalRotation.Roll += CurrentSteerRoll;

    // Apply final rotation to the boat hull. Keep local position at 0
    HullMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    HullMesh->SetRelativeRotation(FinalRotation);
}

void AFishingBoat::EnhancedMove(const FInputActionValue& Value)
{

    if (bIsFishingMode) return;

    FVector2D MovementVector = Value.Get<FVector2D>();
    //float DeltaTime = GetWorld()->GetDeltaSeconds();

    //if (Controller)
    //{
    //    if (MovementVector.Y != 0.0f)
    //    {
    //        FVector DeltaLocation = FVector(MovementVector.Y * MoveSpeed * DeltaTime, 0.f, 0.f);
    //        AddActorLocalOffset(DeltaLocation, true); 
    //    }

    //    if (MovementVector.X != 0.0f)
    //    {
    //        FRotator DeltaRotation = FRotator(0.f, MovementVector.X * TurnSpeed * DeltaTime, 0.f);
    //        AddActorLocalRotation(DeltaRotation, true);
    //    }
    //}

    ForwardInput = MovementVector.Y;
    TurnInput = MovementVector.X;
}

void AFishingBoat::EnhancedMoveStop(const FInputActionValue& Value)
{
    ForwardInput = 0.0f;
    TurnInput = 0.0f;
}

void AFishingBoat::EnhancedLook(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void AFishingBoat::DropBomb()
{
    // Check config
    if (!BoatConfig || !BoatConfig->BombClass)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Bombing failed: BombClass not configured in BoatConfig"));
        return;
    }

    FVector SpawnLocation = GetActorLocation() - (GetActorForwardVector() * 300.f);

    FRotator SpawnRotation = FRotator::ZeroRotator;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this; 
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


    ABombActor* SpawnedBomb = GetWorld()->SpawnActor<ABombActor>(BoatConfig->BombClass, SpawnLocation, SpawnRotation, SpawnParams);

    if (SpawnedBomb)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("C++: Drop bomb success！"));
    }
}

void AFishingBoat::ToggleFishingMode()
{
    bIsFishingMode = !bIsFishingMode;

    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!PC) return;

    if (bIsFishingMode)
    {

        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->RemoveMappingContext(DrivingMappingContext);
            Subsystem->AddMappingContext(FishingMappingContext, 0);
        }

        ForwardInput = 0.0f;
        TurnInput = 0.0f;

        // Active rod components
        for (int32 i = 0; i < AllFishingRods.Num(); i++)
        {
            if (AllFishingRods[i]) 
                AllFishingRods[i]->ActivateRod(i);
        }
    }
    else
    {
        // exit finshing mode
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->RemoveMappingContext(FishingMappingContext);
            Subsystem->AddMappingContext(DrivingMappingContext, 0);
        }

        bIsFeverMode = false;
        ComboCount = 0;

        for (auto& Rod : AllFishingRods)
        {
            if (Rod) Rod->DeactivateRod();
        }
    }
}

void AFishingBoat::TriggerRod1()
{
    DistributeInputToRod(0);
}

void AFishingBoat::TriggerRod2()
{
    DistributeInputToRod(1);
}

void AFishingBoat::TriggerRod3()
{
    DistributeInputToRod(2);
}

void AFishingBoat::TriggerRod4()
{
    DistributeInputToRod(3);
}

void AFishingBoat::DistributeInputToRod(int32 RodIndex)
{

    if (AllFishingRods.IsValidIndex(RodIndex) && AllFishingRods[RodIndex])
    {

        //AllFishingRods[RodIndex]->Interact();
        AllFishingRods[RodIndex]->TryInteract();
    
    }
}

void AFishingBoat::ReportFishingScore(FName Evaluation)
{
    // do not count fever when in fever
    if (bIsFeverMode) return;

    if (Evaluation == FName("PERFECT"))
    {
        ComboCount++;
        if (ComboCount >= BoatConfig->FeverComboThreshold)
        {
            EnterFeverMode();
        }
    }
    //else if (Evaluation == FName("MISS") || Evaluation == FName("PUNISH"))
    else
    {
        // combo back to zero
        ComboCount = 0;
    }
}

void AFishingBoat::EnterFeverMode()
{
    bIsFeverMode = true;
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("FEVER TIME!!"));

    // Fever 10s
    GetWorldTimerManager().SetTimer(FeverTimerHandle, this, &AFishingBoat::ExitFeverMode, BoatConfig->FeverDuration, false);

    OnFeverStateChanged.Broadcast(true);
}

void AFishingBoat::ExitFeverMode()
{
    bIsFeverMode = false;
    ComboCount = 0;
    GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::White, TEXT("Fever End........"));

    OnFeverStateChanged.Broadcast(false);
}

void AFishingBoat::EnterSwarm(AFishSwarmActor* Swarm)
{
    if (Swarm && CurrentSwarm != Swarm)
    {
        CurrentSwarm = Swarm;

        // Broadcast to ui
        OnSwarmStateChanged.Broadcast(CurrentSwarm);


        // Send fish swarm details to rod
        float SwarmMinMult = 1.0f;
        float SwarmMaxMult = 1.0f;

        if (UFishSwarmConfig* SwarmConfig = CurrentSwarm->GetCurrentConfig())
        {
            SwarmMinMult = SwarmConfig->BiteTimeMultiplierMin;
            SwarmMaxMult = SwarmConfig->BiteTimeMultiplierMax;
        }

        for (AFishingRodActor* Rod : AllFishingRods)
        {
            if (Rod) Rod->SetEnvironmentModifiers(SwarmMinMult, SwarmMaxMult);
        }

        // debug
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("C++: Get into the fish area!"));
    }
}

void AFishingBoat::LeaveSwarm(AFishSwarmActor* Swarm)
{
    if (Swarm && CurrentSwarm == Swarm)
    {
        CurrentSwarm = nullptr;

        // broadcast nullptr for leave
        OnSwarmStateChanged.Broadcast(nullptr);

        // Clear fish swarm information to rod
        for (AFishingRodActor* Rod : AllFishingRods)
        {
            if (Rod) Rod->SetEnvironmentModifiers(5.0f, 5.0f);
        }

        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("C++: Leave the fish area"));
    }
}

void AFishingBoat::HandleRodRequestFishData(int32 RodIndex)
{
    if (!AllFishingRods.IsValidIndex(RodIndex) || !AllFishingRods[RodIndex]) return;

    FDataTableRowHandle ChosenFishHandle;

    // Check if there is a fish swarm
    if (CurrentSwarm && CurrentSwarm->HasFishesLeft())
    {
        // Simply draw a fish directly from the current fish population using a weighted algorithm!
        ChosenFishHandle = CurrentSwarm->DrawFishFromPool();
    }
    else
    {
        // If not in fish swarn, can also get some strange things
        //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("There are no fish here."));

        if (BoatConfig && BoatConfig->FallbackTrashPool.Num() > 0)
        {
            float RandomChance = FMath::FRand(); // 0.0 ~ 1.0

            if (RandomChance <= BoatConfig->TrashCatchProbability)
            {

                int32 RandomIndex = FMath::RandRange(0, BoatConfig->FallbackTrashPool.Num() - 1);
                ChosenFishHandle = BoatConfig->FallbackTrashPool[RandomIndex];

                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("Area guarantee: Get a mysterious piece of trash£¡"));
            }
            else
            {
                // bad luck
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Silver, TEXT("Nothing happened, keep waiting"));
            }
        }
    }

    // Put the fish back into the corresponding fishing rod intact.
    AllFishingRods[RodIndex]->AssignFishData(ChosenFishHandle);
}