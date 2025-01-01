using UnrealBuildTool;

public class GameplayExperiencesRuntime : ModuleRules
{
    public GameplayExperiencesRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new[]
        { 
            "Core",
            "GameFeatures",
            "GameFeaturesExtension"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "CoreUObject", 
            "Engine",
        });
    }
}