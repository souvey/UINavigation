// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

using UnrealBuildTool;
using System.IO;

public class UINavigation : ModuleRules
{
	public UINavigation(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrecompileForTargets = PrecompileTargetsType.Any;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"EnhancedInput",
				"ApplicationCore"
			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
                "UMG",
                "Slate",
                "SlateCore",
                "InputCore",
                "RenderCore",
                "EnhancedInput",
                "RHI",
                "HeadMountedDisplay",
				"AssetRegistry",
				"GameplayTags",
			}
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
