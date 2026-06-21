// Fill out your copyright notice in the Description page of Project Settings.

#include "Ocean/OceanManager.h"
#include "../Public/Ocean/OceanConfig.h"
#include "../Shaders/OceanSimulationShaders.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"

AOceanManager::AOceanManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set safe default physical parameters
    DisplacementResolution = 512;
    FFTResolution = 512;
    NumCascades = 4;


    PatchLengths = { 200.0f, 50.0f, 12.5f, 12.5f };
    WaveAmplitudes = { 1.5f, 0.8f, 0.3f,0.3f };

    ChoppyScales = { 0.8f, 1.2f, 2.5f, 2.5f };
    TimeScales = { 0.8f, 1.2f, 1.0f, 1.0f };
    WindAngleOffsets = { 1.0f, 1.2f, 1.0f, 1.0f };
    CascadeWindSpeeds = { 1.0f, 1.0f, 1.0f,1.0f };

    WindSpeed = 15.0f;
    WindDirection = FVector2D(1.0f, 1.0f);

    // Create mesh component
    OceanMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OceanMesh"));

    // Set as Root Component
    RootComponent = OceanMeshComponent;

    OceanMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    OceanMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
}

void AOceanManager::BeginPlay()
{
    if (OceanConfig)
    {
        // Set parameters form config
        SpectrumModel = OceanConfig->SpectrumModel;
        WindSpeed = OceanConfig->WindSpeed;
        WindDirection = OceanConfig->WindDirection;
        WindDependency = OceanConfig->WindDependency;
        Fetch = OceanConfig->Fetch;
        JONSWAP_Gamma = OceanConfig->JONSWAP_Gamma;

        // cascade parameters
        NumCascades = FMath::Clamp(OceanConfig->CascadePresets.Num(), 1, 4);

        // Clear old data
        PatchLengths.Empty(NumCascades);
        WaveAmplitudes.Empty(NumCascades);
        ChoppyScales.Empty(NumCascades);
        TimeScales.Empty(NumCascades);
        WindAngleOffsets.Empty(NumCascades);
        CascadeWindSpeeds.Empty(NumCascades);

        // Set every cascade parameters
        for (int32 i = 0; i < NumCascades; ++i)
        {
            PatchLengths.Add(OceanConfig->CascadePresets[i].PatchLength);
            WaveAmplitudes.Add(OceanConfig->CascadePresets[i].WaveAmplitude);
            ChoppyScales.Add(OceanConfig->CascadePresets[i].ChoppyScale);
            TimeScales.Add(OceanConfig->CascadePresets[i].TimeScale);
            WindAngleOffsets.Add(OceanConfig->CascadePresets[i].WindAngleOffset);
            CascadeWindSpeeds.Add(OceanConfig->CascadePresets[i].CascadeWindSpeed);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("OceanManager: No OceanCondfig! "));

        PatchLengths.SetNumZeroed(NumCascades);
        WaveAmplitudes.SetNumZeroed(NumCascades);
        ChoppyScales.SetNumZeroed(NumCascades);
        TimeScales.SetNumZeroed(NumCascades);
        WindAngleOffsets.SetNumZeroed(NumCascades);
    }

    // [CRITICAL TIMING FIX] 
    // Data initialization MUST precede Super::BeginPlay().
    // Super::BeginPlay() essentially triggers the Blueprint's 'Event BeginPlay'.
    // If called first, the Blueprint will attempt to access Render Targets 
    // before they are allocated in memory, fetching a Null pointer. 

    InitRenderTargets();

    // Material Instantiation & Binding
    if (OceanBaseMaterial && OceanMeshComponent)
    {
        // Create Dynamic Material Instance
        OceanMID = UMaterialInstanceDynamic::Create(OceanBaseMaterial, this);

        if (OceanMID)
        {
            // Inject the computed spectrum texture into material parameter for visual testing
            //OceanMID->SetTextureParameterValue(FName("RT_Output"), H0_Texture);
            //OceanMID->SetTextureParameterValue(FName("RT_Output"), Ht_Texture);
            // OceanMID->SetTextureParameterValue(FName("RT_Output"), DisplacementLUT);

            // OceanMID->SetTextureParameterValue(FName("RT_Normal"), NormalLUT);

            // Apply material to the mesh
            ///OceanMeshComponent->SetMaterial(0, OceanMID);

            OceanMID->SetTextureParameterValue(FName("RT_DisplacementArray"), DisplacementLUTArray);
            OceanMID->SetTextureParameterValue(FName("RT_NormalArray"), NormalLUTArray);
            OceanMeshComponent->SetMaterial(0, OceanMID);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OceanManager: Missing OceanBaseMaterial or Mesh Component is not initialized!"));
    }

    Super::BeginPlay();
}

void AOceanManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Trigger time spectrum update every frame
    DispatchTimeSpectrum(DeltaTime);
}

void AOceanManager::InitRenderTargets()
{

    //(GPU Memory Allocation)
    // 
    // init spatial domain displacement map
    //DisplacementLUT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
    // // Enable Compute Shader Write Access
    //DisplacementLUT->bCanCreateUAV = true; 
    //DisplacementLUT->ClearColor = FLinearColor::Black;
    //DisplacementLUT->InitCustomFormat(DisplacementResolution, DisplacementResolution, PF_A32B32G32R32F, false);
    //DisplacementLUT->UpdateResourceImmediate(true);
    DisplacementLUTArray = NewObject<UTextureRenderTarget2DArray>(this, NAME_None, RF_Transient);
    DisplacementLUTArray->bCanCreateUAV = true;
    DisplacementLUTArray->ClearColor = FLinearColor::Black;
    // Height Width Cascade
    DisplacementLUTArray->Init(DisplacementResolution, DisplacementResolution, NumCascades, PF_A32B32G32R32F);
    DisplacementLUTArray->UpdateResourceImmediate(true);

    // H0_Texture, initial static spectrum, high precision
    
    //H0_Texture = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
    //H0_Texture->bCanCreateUAV = true;
    //H0_Texture->ClearColor = FLinearColor::Black;
    //H0_Texture->InitCustomFormat(FFTResolution, FFTResolution, PF_A32B32G32R32F, false);
    //H0_Texture->UpdateResourceImmediate(true);
    H0_TextureArray = NewObject<UTextureRenderTarget2DArray>(this, NAME_None, RF_Transient);
    H0_TextureArray->bCanCreateUAV = true;
    H0_TextureArray->ClearColor = FLinearColor::Black;
    H0_TextureArray->Init(FFTResolution, FFTResolution, NumCascades, PF_A32B32G32R32F);
    H0_TextureArray->UpdateResourceImmediate(true);

    // init Ht_Texture, time spectrum, Complex numbers only
    
    //Ht_Texture = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
    //Ht_Texture->bCanCreateUAV = true;
    //Ht_Texture->ClearColor = FLinearColor::Black;
    ////Ht_Texture->InitCustomFormat(FFTResolution, FFTResolution, PF_G32R32F, false);
    //Ht_Texture->InitCustomFormat(FFTResolution, FFTResolution, PF_A32B32G32R32F, false);
    //Ht_Texture->UpdateResourceImmediate(true);
    Ht_TextureArray = NewObject<UTextureRenderTarget2DArray>(this, NAME_None, RF_Transient);
    Ht_TextureArray->bCanCreateUAV = true;
    Ht_TextureArray->ClearColor = FLinearColor::Black;
    Ht_TextureArray->Init(FFTResolution, FFTResolution, NumCascades, PF_A32B32G32R32F);
    Ht_TextureArray->UpdateResourceImmediate(true);

    // init Butterfly_Texture
    // width is log2(Resolution) (如 8)，height is Resolution
    int32 Stages = FMath::FloorLog2(FFTResolution);

    Butterfly_Texture = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
    Butterfly_Texture->bCanCreateUAV = true;
    Butterfly_Texture->ClearColor = FLinearColor::Black;
    Butterfly_Texture->InitCustomFormat(Stages, FFTResolution, PF_A32B32G32R32F, false);
    Butterfly_Texture->UpdateResourceImmediate(true);

    // init  Ht_Slope_Texture
    // 
    //Ht_Slope_Texture = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
    //Ht_Slope_Texture->bCanCreateUAV = true;
    //Ht_Slope_Texture->ClearColor = FLinearColor::Black;
    //Ht_Slope_Texture->InitCustomFormat(FFTResolution, FFTResolution, PF_A32B32G32R32F, false);
    //Ht_Slope_Texture->UpdateResourceImmediate(true);
    Ht_Slope_TextureArray = NewObject<UTextureRenderTarget2DArray>(this, NAME_None, RF_Transient);
    Ht_Slope_TextureArray->bCanCreateUAV = true;
    Ht_Slope_TextureArray->ClearColor = FLinearColor::Black;
    Ht_Slope_TextureArray->Init(FFTResolution, FFTResolution, NumCascades, PF_A32B32G32R32F);
    Ht_Slope_TextureArray->UpdateResourceImmediate(true);

    // init NormalLUT, the final normal map used for the material
    //NormalLUT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
    //NormalLUT->bCanCreateUAV = true;
    //NormalLUT->ClearColor = FLinearColor::Black;
    //NormalLUT->InitCustomFormat(FFTResolution, FFTResolution, PF_A32B32G32R32F, false);
    //NormalLUT->UpdateResourceImmediate(true);
    NormalLUTArray = NewObject<UTextureRenderTarget2DArray>(this, NAME_None, RF_Transient);
    NormalLUTArray->bCanCreateUAV = true;
    NormalLUTArray->ClearColor = FLinearColor::Black;
    NormalLUTArray->Init(FFTResolution, FFTResolution, NumCascades, PF_A32B32G32R32F);
    NormalLUTArray->UpdateResourceImmediate(true);

    // execute itial pipeline
    DispatchInitialSpectrum();

    DispatchButterflyTexture();
}

void AOceanManager::DispatchInitialSpectrum()
{
    //if (!H0_Texture || !H0_Texture->GetResource()) return;
    if (!H0_TextureArray || !H0_TextureArray->GetResource()) return;

    //FTextureRenderTargetResource* RTResource = H0_Texture->GameThread_GetRenderTargetResource();
    FTextureRenderTargetResource* RTResource = H0_TextureArray->GameThread_GetRenderTargetResource();

    // Capture physical parameters on Game Thread to prevent data races on Render Thread
    int32 Resolution = FFTResolution;
    int32 Cascades = NumCascades;

    int32 PassSpectrumType = (int32)SpectrumModel;
    float PassFetch = Fetch;
    float PassGamma = JONSWAP_Gamma;

    //float PassPatchLength = PatchLength;
    //float PassAmplitude = WaveAmplitude;
    //Copy parameters
    TArray<float> PassPatchLengths = PatchLengths;
    TArray<float> PassAmplitudes = WaveAmplitudes;
    TArray<float> PassWindAngleOffsets = WindAngleOffsets;
    TArray<float> PassCascadeWindSpeeds = CascadeWindSpeeds;

    float PassWindSpeed = WindSpeed;
    float PassWindDependency = WindDependency;

    // Must be normalized
    FVector2f PassWindDir = FVector2f(WindDirection.GetSafeNormal());

    // Enqueue render command to the Render Thread
    ENQUEUE_RENDER_COMMAND(FOceanInitialSpectrumDispatch)(
        //[RTResource, Resolution, PassPatchLength, PassAmplitude, PassWindSpeed, PassWindDependency, PassWindDir, PassSpectrumType, PassFetch, PassGamma](FRHICommandListImmediate& RHICmdList)
        [RTResource, Resolution, Cascades, PassPatchLengths, PassAmplitudes, PassWindAngleOffsets, PassCascadeWindSpeeds, PassWindSpeed, PassWindDependency, PassWindDir, PassSpectrumType, PassFetch, PassGamma](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);

            // Get Global Shader Map
            TShaderMapRef<FOceanInitialSpectrumCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

            //Register external texture and create UAV
            FRDGTextureRef ExternalTexture = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(RTResource->GetTextureRHI(), TEXT("OceanH0RT")));
            FRDGTextureUAVRef OutputUAV = GraphBuilder.CreateUAV(ExternalTexture);

            // Allocate and fill Shader parameters structure
            FOceanInitialSpectrumCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FOceanInitialSpectrumCS::FParameters>();
            //PassParameters->H0_Texture = OutputUAV;
            PassParameters->H0_TextureArray = OutputUAV;
            PassParameters->Resolution = Resolution;

            PassParameters->NumCascades = Cascades;

            //PassParameters->PatchLength = PassPatchLength;
            //PassParameters->Amplitude = PassAmplitude;

            // Four cascades
            for (int32 i = 0; i < Cascades && i < 4; i++)
            {
                PassParameters->PatchLengths[i] = PassPatchLengths[i];
                PassParameters->Amplitudes[i] = PassAmplitudes[i];
                PassParameters->WindAngleOffsets[i] = PassWindAngleOffsets[i];
                PassParameters->CascadeWindSpeeds[i] = PassCascadeWindSpeeds[i];
            }

            PassParameters->WindSpeed = PassWindSpeed;
            PassParameters->WindDirection = PassWindDir;
            PassParameters->WindDependency = PassWindDependency;
            PassParameters->SpectrumType = PassSpectrumType;
            PassParameters->Fetch = PassFetch;
            PassParameters->JONSWAP_Gamma = PassGamma;

            // Calculate thread group count, 16x16 per group
            uint32 GroupCount = FMath::DivideAndRoundUp(Resolution, 16);
            //FIntVector GroupCounts(GroupCount, GroupCount, 1);
            FIntVector GroupCounts(GroupCount, GroupCount, Cascades);

            // Add Compute Pass to RDG
            FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("OceanInitialSpectrum"), ComputeShader, PassParameters, GroupCounts);

            // Execute
            GraphBuilder.Execute();
        });
}

void AOceanManager::DispatchTimeSpectrum(float DeltaTime)
{

    // Check if required input/output textures are ready
    // 
    //if (!H0_Texture || !H0_Texture->GetResource() || !Ht_Texture || !Ht_Texture->GetResource() ||
    //    !Butterfly_Texture || !Butterfly_Texture->GetResource() || !DisplacementLUT || !DisplacementLUT->GetResource() ||
    //    !Ht_Slope_Texture || !Ht_Slope_Texture->GetResource() || !NormalLUT || !NormalLUT->GetResource()) return;
    if (!H0_TextureArray || !H0_TextureArray->GetResource() || !Ht_TextureArray || !Ht_TextureArray->GetResource() ||
        !Butterfly_Texture || !Butterfly_Texture->GetResource() || !DisplacementLUTArray || !DisplacementLUTArray->GetResource() ||
        !Ht_Slope_TextureArray || !Ht_Slope_TextureArray->GetResource() || !NormalLUTArray || !NormalLUTArray->GetResource()) return;

    //FTextureRenderTargetResource* H0Resource = H0_Texture->GameThread_GetRenderTargetResource();
    //FTextureRenderTargetResource* HtResource = Ht_Texture->GameThread_GetRenderTargetResource();
    //FTextureRenderTargetResource* HtSlopeResource = Ht_Slope_Texture->GameThread_GetRenderTargetResource();
    //FTextureRenderTargetResource* ButterflyResource = Butterfly_Texture->GameThread_GetRenderTargetResource();
    //FTextureRenderTargetResource* DisplacementResource = DisplacementLUT->GameThread_GetRenderTargetResource();
    //FTextureRenderTargetResource* NormalResource = NormalLUT->GameThread_GetRenderTargetResource();
    FTextureRenderTargetResource* H0Resource = H0_TextureArray->GameThread_GetRenderTargetResource();
    FTextureRenderTargetResource* HtResource = Ht_TextureArray->GameThread_GetRenderTargetResource();
    FTextureRenderTargetResource* HtSlopeResource = Ht_Slope_TextureArray->GameThread_GetRenderTargetResource();
    FTextureRenderTargetResource* ButterflyResource = Butterfly_Texture->GameThread_GetRenderTargetResource();
    FTextureRenderTargetResource* DisplacementResource = DisplacementLUTArray->GameThread_GetRenderTargetResource();
    FTextureRenderTargetResource* NormalResource = NormalLUTArray->GameThread_GetRenderTargetResource();

    // Copy pipeline parameters
    //int32 Resolution = FFTResolution;
    //float PassPatchLength = PatchLength;
    // // Get total game time
    //float TotalTime = GetWorld()->GetTimeSeconds();
    //float PassChoppyScale = ChoppyScale;
    int32 Resolution = FFTResolution;
    int32 Cascades = NumCascades;
    TArray<float> PassPatchLengths = PatchLengths;
    TArray<float> PassChoppyScales = ChoppyScales;
    float TotalTime = GetWorld()->GetTimeSeconds();
    TArray<float> PassTimeScales = TimeScales;
    // Correctly capture HtSlopeResource and NormalResource into the Render Thread Lambda
    ENQUEUE_RENDER_COMMAND(FOceanTimeSpectrumDispatch)(
        //[H0Resource, HtResource, HtSlopeResource, ButterflyResource, DisplacementResource, NormalResource, Resolution, PassPatchLength, TotalTime, PassChoppyScale](FRHICommandListImmediate& RHICmdList)
        [H0Resource, HtResource, HtSlopeResource, ButterflyResource, DisplacementResource, NormalResource, Resolution, Cascades, PassPatchLengths, PassChoppyScales, TotalTime, PassTimeScales](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);

            TShaderMapRef<FOceanTimeSpectrumCS> TimeComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

            //// Register input static spectrum H0 - Read Only
            //FRDGTextureRef InputH0 = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(H0Resource->GetTextureRHI(), TEXT("OceanH0")));

            ////Register output dynamic spectrum Ht and create UAV
            //FRDGTextureRef OutputHt = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(HtResource->GetTextureRHI(), TEXT("OceanHt")));
            //FRDGTextureUAVRef OutputHtUAV = GraphBuilder.CreateUAV(OutputHt);

            //// Register new Slope spectrum and create UAV
            //FRDGTextureRef OutputHtSlope = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(HtSlopeResource->GetTextureRHI(), TEXT("OceanHtSlope")));
            //FRDGTextureUAVRef OutputHtSlopeUAV = GraphBuilder.CreateUAV(OutputHtSlope);

            //// Fill time step parameters
            //FOceanTimeSpectrumCS::FParameters* TimePassParams = GraphBuilder.AllocParameters<FOceanTimeSpectrumCS::FParameters>();
            //TimePassParams->H0_Texture = InputH0;
            //TimePassParams->Ht_Texture = OutputHtUAV;
            //TimePassParams->Ht_Slope_Texture = OutputHtSlopeUAV; // [新增] 绑定斜率输出 (Bind slope output)
            //TimePassParams->Resolution = Resolution;
            //TimePassParams->PatchLength = PassPatchLength;
            //TimePassParams->Time = TotalTime;

            // TextureArray
            FRDGTextureRef InputH0 = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(H0Resource->GetTextureRHI(), TEXT("OceanH0Array")));
            FRDGTextureRef OutputHt = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(HtResource->GetTextureRHI(), TEXT("OceanHtArray")));
            FRDGTextureRef OutputHtSlope = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(HtSlopeResource->GetTextureRHI(), TEXT("OceanHtSlopeArray")));

            FOceanTimeSpectrumCS::FParameters* TimePassParams = GraphBuilder.AllocParameters<FOceanTimeSpectrumCS::FParameters>();
            TimePassParams->Resolution = Resolution;
            TimePassParams->H0_TextureArray = InputH0;
            TimePassParams->Ht_TextureArray = GraphBuilder.CreateUAV(OutputHt);
            TimePassParams->Ht_Slope_TextureArray = GraphBuilder.CreateUAV(OutputHtSlope);
            // casacdes
            for (int32 i = 0; i < Cascades && i < 4; i++) {
                TimePassParams->PatchLengths[i] = PassPatchLengths[i];
                TimePassParams->TimeScales[i] = PassTimeScales[i];
            }
            TimePassParams->Time = TotalTime;

            uint32 GroupCount = FMath::DivideAndRoundUp(Resolution, 16);
            //FIntVector GroupCounts(GroupCount, GroupCount, 1);
            FIntVector GroupCounts(GroupCount, GroupCount, Cascades);

            // Dispatch Time Rotation Compute Pass
            FComputeShaderUtils::AddPass(
                GraphBuilder,
                RDG_EVENT_NAME("OceanTimeSpectrum"),
                TimeComputeShader,
                TimePassParams,
                GroupCounts
            );

            // Dual-Pipeline IFFT Memory Setup
            // GEt IFFT Shader
            TShaderMapRef<FOceanIFFTCS> IFFTShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

            // resigter Butterfly texture
            FRDGTextureRef ButterflyTex = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(ButterflyResource->GetTextureRHI(), TEXT("ButterflyRT")));

            // Create a temporary Ping-Pong texture in the video memory. A PF_A32B32G32R32F microcontroller with 4 channels must be used.
            //FRDGTextureDesc TempDesc = FRDGTextureDesc::Create2D(FIntPoint(Resolution, Resolution), PF_A32B32G32R32F, FClearValueBinding::None, TexCreate_ShaderResource | TexCreate_UAV);
            FRDGTextureDesc TempDesc = FRDGTextureDesc::Create2DArray(FIntPoint(Resolution, Resolution), PF_A32B32G32R32F, FClearValueBinding::None, TexCreate_ShaderResource | TexCreate_UAV, Cascades);

            // Pipeline A: Displacement data Ping-Pong RTs
            FRDGTextureRef PingRT = GraphBuilder.CreateTexture(TempDesc, TEXT("OceanPing"));
            FRDGTextureRef PongRT = GraphBuilder.CreateTexture(TempDesc, TEXT("OceanPong"));
            FRDGTextureRef CurrentInput = OutputHt;
            FRDGTextureRef CurrentOutput = PingRT;

            // Pipeline B: Slope data Ping-Pong RTs
            FRDGTextureRef PingSlopeRT = GraphBuilder.CreateTexture(TempDesc, TEXT("OceanPingSlope"));
            FRDGTextureRef PongSlopeRT = GraphBuilder.CreateTexture(TempDesc, TEXT("OceanPongSlope"));
            FRDGTextureRef CurrentInputSlope = OutputHtSlope;
            FRDGTextureRef CurrentOutputSlope = PingSlopeRT;

            int32 Stages = FMath::FloorLog2(Resolution);

            // Horizontal 1D FFT
            for (int32 i = 0; i < Stages; i++)
            {
                // Pipeline A: Height and Choppy displacement
                FOceanIFFTCS::FParameters* IFFTParams = GraphBuilder.AllocParameters<FOceanIFFTCS::FParameters>();
                IFFTParams->Butterfly_Texture = ButterflyTex;
                //IFFTParams->Input_Texture = CurrentInput;
                //IFFTParams->Output_Texture = GraphBuilder.CreateUAV(CurrentOutput);
                IFFTParams->Input_TextureArray = CurrentInput;
                IFFTParams->Output_TextureArray = GraphBuilder.CreateUAV(CurrentOutput);
                IFFTParams->Stage = i;
                IFFTParams->Direction = 0; // Horizontal
                IFFTParams->Resolution = Resolution;
                FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("IFFT_Horz_Height_Stage_%d", i), IFFTShader, IFFTParams, GroupCounts);

                // Pipeline B: Slope X and Slope Y
                FOceanIFFTCS::FParameters* IFFTParamsSlope = GraphBuilder.AllocParameters<FOceanIFFTCS::FParameters>();
                IFFTParamsSlope->Butterfly_Texture = ButterflyTex;
                //IFFTParamsSlope->Input_Texture = CurrentInputSlope;
                //IFFTParamsSlope->Output_Texture = GraphBuilder.CreateUAV(CurrentOutputSlope);
                IFFTParamsSlope->Input_TextureArray = CurrentInputSlope;
                IFFTParamsSlope->Output_TextureArray = GraphBuilder.CreateUAV(CurrentOutputSlope);
                IFFTParamsSlope->Stage = i;
                IFFTParamsSlope->Direction = 0;
                IFFTParamsSlope->Resolution = Resolution;
                FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("IFFT_Horz_Slope_Stage_%d", i), IFFTShader, IFFTParamsSlope, GroupCounts);

                // Double Ping-Pong Swap
                FRDGTextureRef Temp = CurrentInput;
                CurrentInput = CurrentOutput;
                CurrentOutput = (Temp == OutputHt) ? PongRT : Temp;

                FRDGTextureRef TempSlope = CurrentInputSlope;
                CurrentInputSlope = CurrentOutputSlope;
                CurrentOutputSlope = (TempSlope == OutputHtSlope) ? PongSlopeRT : TempSlope;
            }

            //Vertical 1D FFT
            for (int32 i = 0; i < Stages; i++)
            {
                // Pipeline A: Height and Choppy displacement
                FOceanIFFTCS::FParameters* IFFTParams = GraphBuilder.AllocParameters<FOceanIFFTCS::FParameters>();
                IFFTParams->Butterfly_Texture = ButterflyTex;
                IFFTParams->Input_TextureArray = CurrentInput;
                IFFTParams->Output_TextureArray = GraphBuilder.CreateUAV(CurrentOutput);
                IFFTParams->Stage = i;
                IFFTParams->Direction = 1; // Vertical
                IFFTParams->Resolution = Resolution;
                FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("IFFT_Vert_Height_Stage_%d", i), IFFTShader, IFFTParams, GroupCounts);

                //Pipeline B: Slope X and Slope Y
                FOceanIFFTCS::FParameters* IFFTParamsSlope = GraphBuilder.AllocParameters<FOceanIFFTCS::FParameters>();
                IFFTParamsSlope->Butterfly_Texture = ButterflyTex;
                IFFTParamsSlope->Input_TextureArray = CurrentInputSlope;
                IFFTParamsSlope->Output_TextureArray = GraphBuilder.CreateUAV(CurrentOutputSlope);
                IFFTParamsSlope->Stage = i;
                IFFTParamsSlope->Direction = 1;
                IFFTParamsSlope->Resolution = Resolution;
                FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("IFFT_Vert_Slope_Stage_%d", i), IFFTShader, IFFTParamsSlope, GroupCounts);

                // Double Ping-Pong Swap
                FRDGTextureRef Temp = CurrentInput;
                CurrentInput = CurrentOutput;
                CurrentOutput = (Temp == OutputHt) ? PongRT : Temp;

                FRDGTextureRef TempSlope = CurrentInputSlope;
                CurrentInputSlope = CurrentOutputSlope;
                CurrentOutputSlope = (TempSlope == OutputHtSlope) ? PongSlopeRT : TempSlope;
            }

            // Assemble
            TShaderMapRef<FOceanAssembleCS> AssembleShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            FRDGTextureRef FinalDisplacement = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(DisplacementResource->GetTextureRHI(), TEXT("FinalDisplacement")));

            // Register analytical normal output target
            FRDGTextureRef FinalNormal = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(NormalResource->GetTextureRHI(), TEXT("FinalNormal")));

            FOceanAssembleCS::FParameters* AssembleParams = GraphBuilder.AllocParameters<FOceanAssembleCS::FParameters>();
            //AssembleParams->IFFT_Result = CurrentInput;
            //AssembleParams->IFFT_Slope_Result = CurrentInputSlope; 
            //AssembleParams->DisplacementLUT = GraphBuilder.CreateUAV(FinalDisplacement);
            //AssembleParams->NormalLUT = GraphBuilder.CreateUAV(FinalNormal);
            AssembleParams->IFFT_ResultArray = CurrentInput;
            AssembleParams->IFFT_Slope_ResultArray = CurrentInputSlope;
            AssembleParams->DisplacementLUTArray = GraphBuilder.CreateUAV(FinalDisplacement);
            AssembleParams->NormalLUTArray = GraphBuilder.CreateUAV(FinalNormal);
            AssembleParams->Resolution = Resolution;
            //AssembleParams->ChoppyScale = PassChoppyScale;
            for (int32 i = 0; i < Cascades && i < 4; i++) {
                AssembleParams->ChoppyScales[i] = PassChoppyScales[i];
            }

            FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("OceanAssemble"), AssembleShader, AssembleParams, GroupCounts);

            // Execute the Render Graph
            GraphBuilder.Execute();
        });
}

void AOceanManager::DispatchButterflyTexture()
{
    if (!Butterfly_Texture || !Butterfly_Texture->GetResource()) return;

    FTextureRenderTargetResource* RTResource = Butterfly_Texture->GameThread_GetRenderTargetResource();

    int32 Resolution = FFTResolution;

    ENQUEUE_RENDER_COMMAND(FOceanButterflyDispatch)(
        [RTResource, Resolution](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);

            TShaderMapRef<FOceanButterflyTextureCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

            FRDGTextureRef ExternalTexture = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(RTResource->GetTextureRHI(), TEXT("OceanButterflyRT")));
            FRDGTextureUAVRef OutputUAV = GraphBuilder.CreateUAV(ExternalTexture);

            // add parameter
            FOceanButterflyTextureCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FOceanButterflyTextureCS::FParameters>();
            PassParameters->Butterfly_Texture = OutputUAV;
            PassParameters->Resolution = Resolution;

            // Calculate thread group count
            uint32 Stages = FMath::FloorLog2(Resolution);

            uint32 GroupCountY = FMath::DivideAndRoundUp(Resolution, 16);
            FIntVector GroupCounts(Stages, GroupCountY, 1);

            FComputeShaderUtils::AddPass(
                GraphBuilder,
                RDG_EVENT_NAME("OceanButterflyTexture"),
                ComputeShader,
                PassParameters,
                GroupCounts
            );

            GraphBuilder.Execute();
        });
}

float AOceanManager::GetApproximateOceanHeight(FVector WorldLocation)
{
    // Check if ocean config exists. If not, return default base Z level
    if (!OceanConfig || OceanConfig->CascadePresets.Num() == 0)
    {
        return GetActorLocation().Z;
    }

    // Get wind direction and convert UE units (cm) to meters
    FVector2D WindDir = OceanConfig->WindDirection.GetSafeNormal();
    FVector2D Pos_m(WorldLocation.X * 0.01f, WorldLocation.Y * 0.01f);

    float BaseTime = GetWorld()->GetTimeSeconds();
    float TotalHeight_m = 0.0f;
    float CPUHeightScalar = 0.25f; // Scale down CPU wave height to match GPU visual

    int32 Cascades = OceanConfig->CascadePresets.Num();

    // Loop through wave layers. Start from 1 to skip the biggest wave (Cascade 0)
    for (int32 i = 1; i < FMath::Min(3, Cascades); i++)
    {
        float PatchLength = OceanConfig->CascadePresets[i].PatchLength;
        float EnergyAmplitude = OceanConfig->CascadePresets[i].WaveAmplitude;
        float CurrentTimeScale = OceanConfig->CascadePresets[i].TimeScale;

        // Only calculate if wave length is valid
        if (PatchLength > 0.01f)
        {
            // Math for the main wave
            float k1 = (2.0f * PI) / PatchLength;
            float omega1 = FMath::Sqrt(9.81f * k1);

            FVector2D CurrentWindDir = WindDir;
            // Rotate wind direction a bit for cascade 1 to look more random
            if (i == 1)
            {
                CurrentWindDir = FVector2D(WindDir.X * 0.9f - WindDir.Y * 0.43f, WindDir.X * 0.43f + WindDir.Y * 0.9f).GetSafeNormal();
            }

            // Calculate main wave movement over time
            float phase1 = k1 * FVector2D::DotProduct(CurrentWindDir, Pos_m) - omega1 * (BaseTime * CurrentTimeScale);

            // Create side wave 1 to break the perfect wave shape
            FVector2D Dir2(CurrentWindDir.X * 0.866f - CurrentWindDir.Y * 0.5f, CurrentWindDir.X * 0.5f + CurrentWindDir.Y * 0.866f);
            float k2 = (2.0f * PI) / (PatchLength * 0.8f);
            float omega2 = FMath::Sqrt(9.81f * k2);
            float phase2 = k2 * FVector2D::DotProduct(Dir2.GetSafeNormal(), Pos_m) - omega2 * (BaseTime * CurrentTimeScale) + 1.2f;

            // Create side wave 2
            FVector2D Dir3(CurrentWindDir.X * 0.866f + CurrentWindDir.Y * 0.5f, -CurrentWindDir.X * 0.5f + CurrentWindDir.Y * 0.866f);
            float k3 = (2.0f * PI) / (PatchLength * 0.9f);
            float omega3 = FMath::Sqrt(9.81f * k3);
            float phase3 = k3 * FVector2D::DotProduct(Dir3.GetSafeNormal(), Pos_m) - omega3 * (BaseTime * CurrentTimeScale) + 2.5f;

            // Mix the three waves together
            float RawMathHeight = (0.6f * FMath::Cos(phase1)) + (0.2f * FMath::Cos(phase2)) + (0.2f * FMath::Cos(phase3));

            TotalHeight_m += EnergyAmplitude * RawMathHeight * CPUHeightScalar;
        }
    }

    // Convert meters back to UE units (cm) and add base ocean Z position
    return GetActorLocation().Z + (TotalHeight_m * 100.0f);
}

