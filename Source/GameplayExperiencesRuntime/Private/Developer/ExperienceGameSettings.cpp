// Copyright © 2024 Playton. All Rights Reserved.


#include "Developer/ExperienceGameSettings.h"

#include "ExperienceAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceGameSettings)

UExperienceGameSettings::UExperienceGameSettings()
{
	StateChain.Reserve(4);
	StateChain.Add(FGameplayTag());
	StateChain.Add(FGameplayTag());
	StateChain.Add(FGameplayTag());
	StateChain.Add(FGameplayTag());
}

UExperienceGameSettings* UExperienceGameSettings::Get()
{
	return GetMutableDefault<UExperienceGameSettings>();
}

FName UExperienceGameSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

#if WITH_EDITOR
FText UExperienceGameSettings::GetSectionText() const
{
	return NSLOCTEXT("ExperienceDeveloperSettings", "ExperienceDeveloperSettings", "Gameplay Experiences");
}
#endif

void UExperienceGameSettings::OverrideConfigSection(FString& InOutSectionName)
{
	// Mirror the exact properties
	InOutSectionName = TEXT("/Script/GameplayExperiencesRuntime.ExperienceManagerSubsystem");
}

#if WITH_EDITOR
void UExperienceGameSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ApplySettings();
}

void UExperienceGameSettings::ApplySettings()
{
}
#endif
