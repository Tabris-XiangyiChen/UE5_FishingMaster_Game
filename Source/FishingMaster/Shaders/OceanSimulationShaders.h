#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterMacros.h"
#include "DataDrivenShaderPlatformInfo.h"

class FISHINGMASTER_API FOceanSimulationCS : public FGlobalShader
{
    // Declare
    DECLARE_GLOBAL_SHADER(FOceanSimulationCS);
    SHADER_USE_PARAMETER_STRUCT(FOceanSimulationCS, FGlobalShader);

    // Parameters
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
        SHADER_PARAMETER(int32, Resolution)
        SHADER_PARAMETER(float, Time)
    END_SHADER_PARAMETER_STRUCT()

public:
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

class FISHINGMASTER_API FOceanInitialSpectrumCS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FOceanInitialSpectrumCS);
    SHADER_USE_PARAMETER_STRUCT(FOceanInitialSpectrumCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        // SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, H0_Texture)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2DArray<float4>, H0_TextureArray)

        SHADER_PARAMETER(int32, Resolution)
        SHADER_PARAMETER(int32, NumCascades)

        // SHADER_PARAMETER(float, PatchLength)
        // SHADER_PARAMETER(float, Amplitude)
        //SHADER_PARAMETER_SCALAR_ARRAY(float, PatchLengths, [4])
        //SHADER_PARAMETER_SCALAR_ARRAY(float, Amplitudes, [4])
        SHADER_PARAMETER(FVector4f, PatchLengths)
        SHADER_PARAMETER(FVector4f, Amplitudes)
        SHADER_PARAMETER(FVector4f, WindAngleOffsets)
        SHADER_PARAMETER(FVector4f, CascadeWindSpeeds)

        SHADER_PARAMETER(float, WindSpeed)
        SHADER_PARAMETER(FVector2f, WindDirection)
        SHADER_PARAMETER(float, WindDependency)
        SHADER_PARAMETER(int32, SpectrumType)
        SHADER_PARAMETER(float, Fetch)
        SHADER_PARAMETER(float, JONSWAP_Gamma)
    END_SHADER_PARAMETER_STRUCT()

public:
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

// TimeSpectrum Shader
class FISHINGMASTER_API FOceanTimeSpectrumCS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FOceanTimeSpectrumCS);
    SHADER_USE_PARAMETER_STRUCT(FOceanTimeSpectrumCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        // SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, H0_Texture)
        // SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Ht_Texture)
        // SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Ht_Slope_Texture)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2DArray<float4>, H0_TextureArray)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2DArray<float4>, Ht_TextureArray)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2DArray<float4>, Ht_Slope_TextureArray)

        SHADER_PARAMETER(int32, Resolution)

        // SHADER_PARAMETER(float, PatchLength)
        //SHADER_PARAMETER_SCALAR_ARRAY(float, PatchLengths, [4])
        SHADER_PARAMETER(FVector4f, PatchLengths)

        SHADER_PARAMETER(float, Time)
        SHADER_PARAMETER(FVector4f, TimeScales)
    END_SHADER_PARAMETER_STRUCT()

public:
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

// Butterfly texture
class FISHINGMASTER_API FOceanButterflyTextureCS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FOceanButterflyTextureCS);
    SHADER_USE_PARAMETER_STRUCT(FOceanButterflyTextureCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )

        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Butterfly_Texture)
        SHADER_PARAMETER(int32, Resolution)
    END_SHADER_PARAMETER_STRUCT()

public:
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

// IFFT shader
class FISHINGMASTER_API FOceanIFFTCS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FOceanIFFTCS);
    SHADER_USE_PARAMETER_STRUCT(FOceanIFFTCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, Butterfly_Texture)

        // SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, Input_Texture)
        // SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output_Texture)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2DArray<float4>, Input_TextureArray)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2DArray<float4>, Output_TextureArray)

        SHADER_PARAMETER(int32, Stage)
        SHADER_PARAMETER(int32, Direction)
        SHADER_PARAMETER(int32, Resolution)
    END_SHADER_PARAMETER_STRUCT()
public:
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5); }
};

// Sort shader
class FISHINGMASTER_API FOceanAssembleCS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FOceanAssembleCS);
    SHADER_USE_PARAMETER_STRUCT(FOceanAssembleCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        // SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, IFFT_Result)

        // SHADER_PARAMETER_RDG_TEXTURE(Texture2D<float4>, IFFT_Slope_Result)
        // SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, DisplacementLUT)
        // SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, NormalLUT)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2DArray<float4>, IFFT_ResultArray)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2DArray<float4>, IFFT_Slope_ResultArray)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2DArray<float4>, DisplacementLUTArray)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2DArray<float4>, NormalLUTArray)

        SHADER_PARAMETER(int32, Resolution)

        // SHADER_PARAMETER(float, ChoppyScale)
        //SHADER_PARAMETER_SCALAR_ARRAY(float, ChoppyScales, [4])
        SHADER_PARAMETER(FVector4f, ChoppyScales)
    END_SHADER_PARAMETER_STRUCT()
public:
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5); }
};