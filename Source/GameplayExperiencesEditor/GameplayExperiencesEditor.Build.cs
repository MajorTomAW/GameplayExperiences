using UnrealBuildTool;

public class GameplayExperiencesEditor : ModuleRules
{
    public GameplayExperiencesEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "GameFeatures",
                "GameplayExperiencesRuntime",
                "GameFeaturesExtensionEditor",
                "UnrealEd",
                "AssetDefinition",
            }
        );
    }
}