#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "FishData.h"
#include "FishingBoat.generated.h"

class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AFishingRodActor;
class AOceanManager;
class USphereComponent;
class UBombConfig;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFeverStateChanged, bool, bIsFever);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwarmStateChanged, AFishSwarmActor*, ActiveSwarm);

class UFishingBoatConfig;
class AFishSwarmActor;
//class USceneComponent

UCLASS()
class FISHINGMASTER_API AFishingBoat : public APawn
{
    GENERATED_BODY()

public:
    AFishingBoat();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnFeverStateChanged OnFeverStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSwarmStateChanged OnSwarmStateChanged;

    void EnterSwarm(AFishSwarmActor* Swarm);
    void LeaveSwarm(AFishSwarmActor* Swarm);

    UFUNCTION()
    void HandleRodRequestFishData(int32 RodIndex);

    // get current swarm
    AFishSwarmActor* GetCurrentSwarm() const { return CurrentSwarm; }

    // Buoyancy and Ocean Interaction System
    // Pointer to the ocean manager in the scene (obtain in Blueprint or BeginPlay)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
    AOceanManager* OceanManager;

    // Boat waterline offset (unit: cm). Increase to float higher, decrease to sink lower.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
    float WaterLineOffset = -50.0f;

    // Ocean damping: smaller values make the boat rock more violently, larger values make it feel heavier.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
    float BuoyancyDamping = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
    float PitchMultiplier = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
    float RollMultiplier = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
    float TurnRotateSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bIsMainMenuProp = false;

protected:
    virtual void BeginPlay() override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr< UStaticMeshComponent> HullMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> CraneMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> CatchFishZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr <UStaticMeshComponent> PropellerMesh1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> PropellerMesh2;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> FishStoragePoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Rods")
    TObjectPtr<UChildActorComponent> RodSlot1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Rods")
    TObjectPtr<UChildActorComponent> RodSlot2;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Rods")
    TObjectPtr<UChildActorComponent> RodSlot3;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Rods")
    TObjectPtr<UChildActorComponent> RodSlot4;

    UPROPERTY()
    TArray<TObjectPtr<AFishingRodActor>> AllFishingRods;

    // Camera
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<USpringArmComponent> SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> ViewCamera;

    // Camera & Handling Config
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    TObjectPtr<UFishingBoatConfig> BoatConfig;

    // Engine Audio Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<UAudioComponent> EngineAudioComp;

    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    //TObjectPtr<UBombConfig> BombConfig;


    // Enhanced Input
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Driving")
    TObjectPtr<UInputMappingContext> DrivingMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Fishing")
    TObjectPtr<UInputMappingContext> FishingMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Driving")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Driving")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Driving")
    TObjectPtr <UInputAction> BombAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Driving")
    TObjectPtr <UInputAction> ToggleFishingModeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Fishing")
    TObjectPtr<UInputAction> TriggerRod1Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Fishing")
    TObjectPtr<UInputAction> TriggerRod2Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Fishing")
    TObjectPtr<UInputAction> TriggerRod3Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Fishing")
    TObjectPtr<UInputAction> TriggerRod4Action;

    // Overlap callback
    UFUNCTION()
    void OnBoatOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnBoatOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
    
    UFUNCTION()
    void OnCatchFishZoneOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void HandleFishCaughtSuccess(FDataTableRowHandle CaughtFishRow, FVector SpawnLocation);


    // Calculate approximate ocean height at a given world location using CPU math.
    //float GetApproximateOceanHeight(FVector WorldLocation);

    void CalculateBuoyancy(float DeltaTime);


    // Driving state
    float CurrentForwardSpeed = 0.0f;
    float CurrentTurnSpeed = 0.0f;
    // -1 to 1
    float ForwardInput = 0.0f;
    float TurnInput = 0.0f;

    // Fishing State
    bool bIsFishingMode = false;
    bool bIsFeverMode = false;
    int32 ComboCount = 0;

    // Independent state variables for decoupled physics simulation
    FRotator CurrentWaveRotation = FRotator::ZeroRotator;
    float CurrentSteerRoll = 0.0f;

private:
    // Remeber current swarm
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fishing State", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AFishSwarmActor> CurrentSwarm;

    void EnhancedMove(const FInputActionValue& Value);
    void EnhancedMoveStop(const FInputActionValue& Value);


    void EnhancedLook(const FInputActionValue& Value);
    void DropBomb();
    void ToggleFishingMode();

    void TriggerRod1();
    void TriggerRod2();
    void TriggerRod3();
    void TriggerRod4();

    void DistributeInputToRod(int32 index);

    UFUNCTION()
    void ReportFishingScore(FName Evaluation);
    void EnterFeverMode();
    void ExitFeverMode();

    FTimerHandle FeverTimerHandle;
};