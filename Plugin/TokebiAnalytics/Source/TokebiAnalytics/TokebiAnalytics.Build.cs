using UnrealBuildTool;

public class TokebiAnalytics : ModuleRules
{
    public TokebiAnalytics(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine"
            }
        );
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "HTTP",
                "Json",
                "JsonUtilities",
                "Settings",
                "Projects"
            }
        );
    }
}