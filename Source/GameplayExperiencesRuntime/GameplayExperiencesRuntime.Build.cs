using UnrealBuildTool;

public class GameplayExperiencesRuntime : ModuleRules
{
    public GameplayExperiencesRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new[]
        { 
            "Core",
            "SlateCore",
            "GameFeatures",
            "GameFeaturesExtension",
            "GameplayTags",
            "ModularGameplay",
            "ModularGameplayActors",
            "EnhancedInput",
            "DeveloperSettings",
            "GameplayAbilities",
            "ModularGameplayAbilities",
            "CommonLoadingScreen"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "CoreUObject", 
            "Engine",
            "NetCore",
        });
    }
}