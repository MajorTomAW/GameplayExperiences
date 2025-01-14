using UnrealBuildTool;

public class GameplayExperiencesRuntime : ModuleRules
{
    public GameplayExperiencesRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new[]
        { 
            "Core",
            "CoreUObject",
            "Engine",
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
            "CommonLoadingScreen",
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "CoreUObject", 
            "NetCore",
        });
    }
}