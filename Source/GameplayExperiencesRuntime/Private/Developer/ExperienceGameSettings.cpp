// Copyright © 2024 Playton. All Rights Reserved.


#include "Developer/ExperienceGameSettings.h"

#include "ExperienceAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceGameSettings)

UExperienceGameSettings::UExperienceGameSettings()
{
}

UExperienceGameSettings* UExperienceGameSettings::Get()
{
	return GetMutableDefault<UExperienceGameSettings>();
}

FName UExperienceGameSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

FText UExperienceGameSettings::GetSectionText() const
{
	return NSLOCTEXT("ExperienceDeveloperSettings", "ExperienceDeveloperSettings", "Gameplay Experiences");
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
