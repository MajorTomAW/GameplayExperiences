using UnrealBuildTool;

public class GameplayExperiencesRuntime : ModuleRules
{
    public GameplayExperiencesRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new[]
        { 
            "Core",
            "GameFeatures",
            "GameFeaturesExtension",
            "ModularGameplay",
            "ModularGameplayActors",
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "CoreUObject", 
            "Engine",
            "NetCore",
        });
    }
}