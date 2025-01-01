#include "ExperienceDefinition.h"
#include "GameFeaturesExtensionEditor.h"
#include "Engine/AssetManager.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FGameplayExperiencesEditorModule"

class FGameplayExperiencesEditorModule : public FDefaultModuleImpl
{
	virtual void StartupModule() override
	{
		UAssetManager::CallOrRegister_OnAssetManagerCreated(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGameplayExperiencesEditorModule::OnAssetManagerCreated));
	}
	

	void OnAssetManagerCreated()
	{
		// Make sure the game has the appropriate asset manager configuration, or we won't be able to load game feature data assets
		FGameFeaturesExtensionEditorModule& ExtensionModule = FModuleManager::LoadModuleChecked<FGameFeaturesExtensionEditorModule>("GameFeaturesExtensionEditor");

		
		{ // UExperienceDefinition
			FPrimaryAssetTypeInfo TypeInfo;
			TypeInfo.bHasBlueprintClasses = true;
			ExtensionModule.CheckPrimaryAssetDataRule(UExperienceDefinition::StaticClass(), TypeInfo);
		}
	}
};

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGameplayExperiencesEditorModule, GameplayExperiencesEditor)