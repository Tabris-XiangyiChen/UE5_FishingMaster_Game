// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FishingMaster : ModuleRules
{
	public FishingMaster(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "RenderCore", "RHI", "Projects", "Niagara", "UMG", "Slate", "SlateCore" });
	}
}
