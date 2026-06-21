// Copyright Epic Games, Inc. All Rights Reserved.

#include "FishingMaster.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"
#include "Interfaces/IPluginManager.h"

class FFishingMasterGameModule : public FDefaultGameModuleImpl
{
    virtual void StartupModule() override
    {
        // Get the project's actual physical path
        //FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
        //FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("FishingMaster"), TEXT("Shaders"));

        //FString ShaderDirectory = TEXT("C:/Users/u5752303/Desktop/UE5_FishingMaster/Source/FishingMaster/Shaders");

        FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("FishingMaster"), TEXT("Shaders"));
        //FString TestFilePath = FPaths::Combine(ShaderDirectory, TEXT("OceanSimulation.usf"));

        // Map physical paths to virtual paths "/Project/Shaders"
        AddShaderSourceDirectoryMapping(TEXT("/FishingMasterShaders"), ShaderDirectory);
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE( FFishingMasterGameModule, FishingMaster, "FishingMaster" );
 