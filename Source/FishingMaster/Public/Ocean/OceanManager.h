#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureRenderTarget2DArray.h"
#include "OceanManager.generated.h"

UENUM(BlueprintType)
enum class EOceanSpectrumType : uint8
{
    Phillips = 0 UMETA(DisplayName = "Phillips"),
    JONSWAP = 1 UMETA(DisplayName = "JONSWAP")
};

class UOceanConfig;

UCLASS()
class FISHINGMASTER_API AOceanManager : public AActor
{
    GENERATED_BODY()

public:
    AOceanManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Ocean config reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|Config")
    TObjectPtr<UOceanConfig> OceanConfig;

    // Get approximate CPU water height at a given world location
    UFUNCTION(BlueprintCallable, Category = "Ocean|Physics")
    float GetApproximateOceanHeight(FVector WorldLocation);


    // Components  Materials
    // Physical mesh component of the ocean
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ocean|Components")
    class UStaticMeshComponent* OceanMeshComponent;

    // Base material slot for editor assignment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|Material")
    class UMaterialInterface* OceanBaseMaterial;

    // Dynamically generated material instance at runtime
    UPROPERTY(Transient)
    class UMaterialInstanceDynamic* OceanMID;


    // Render Targets / GPU Memory Outputs
    //Initial static spectrum, generated once
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Ocean|Output")
    UTextureRenderTarget2DArray* H0_TextureArray;

    //Time-dependent dynamic spectrum, updated per frame
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Ocean|Output")
    UTextureRenderTarget2DArray* Ht_TextureArray;

    // Butterfly operation codebook, generated only once during initialization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Ocean|Output")
    UTextureRenderTarget2D* Butterfly_Texture;

    // Final spatial domain displacement map
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Ocean|Output")
    UTextureRenderTarget2DArray* DisplacementLUTArray;

    // Frequency domain slope map
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Ocean|Output")
    UTextureRenderTarget2DArray* Ht_Slope_TextureArray;

    // Final Analytical Normal map
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Ocean|Output")
    UTextureRenderTarget2DArray* NormalLUTArray;

    // FFT Grid Parameters
    //Resolution of the final displacement map
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Grid")
    int32 DisplacementResolution = 256;

    // Frequency domain grid size
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Grid")
    int32 FFTResolution = 256;

    // number of cascades
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Grid", meta = (ClampMin = "1", ClampMax = "4"))
    int32 NumCascades = 3;

    /*
    // Physical patch lengths for each cascade
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Physics")
    TArray<float> PatchLengths;

    // Physics & Weather Parameters
    // Selection of mathematical spectrum model
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Physics")
    EOceanSpectrumType SpectrumModel = EOceanSpectrumType::JONSWAP;

    // Global wave amplitude modifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Physics")
    TArray<float> WaveAmplitudes;

    // Wind speed in m/s
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Physics")
    float WindSpeed = 12.0f;

    //2D Wind direction vector
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Physics")
    FVector2D WindDirection = FVector2D(1.0f, 0.5f);

    // Damping factor to truncate ultra-high frequency waves
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Physics")
    float WindDependency = 0.005f;

    // Fetch length in km
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Physics")
    float Fetch = 200.0f;

    // Peak enhancement factor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Physics")
    float JONSWAP_Gamma = 5.5f;

    // Independent choppy scales
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ocean|FFT|Physics")
    TArray<float> ChoppyScales;
    */
private:

    //Physically isolated memory regions read by the runtime rendering pipeline
    TArray<float> PatchLengths;
    TArray<float> WaveAmplitudes;
    TArray<float> ChoppyScales;
    TArray<float> TimeScales;
    TArray<float> WindAngleOffsets;
    TArray<float> CascadeWindSpeeds;

    EOceanSpectrumType SpectrumModel;
    float WindSpeed;
    FVector2D WindDirection;
    float WindDependency;
    float Fetch;
    float JONSWAP_Gamma;



    // Initialize  GPU render targets memory
    void InitRenderTargets();

    // Dispatch initial spectrum calculation, runs once
    void DispatchInitialSpectrum();

    // Dispatch time-dependent spectrum calculation, runs per frame
    void DispatchTimeSpectrum(float DeltaTime);

    // Dispatch Butterfly Texture Generation
    void DispatchButterflyTexture();
};