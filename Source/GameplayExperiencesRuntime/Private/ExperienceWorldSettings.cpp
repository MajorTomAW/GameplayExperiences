// Copyright © 2024 Playton. All Rights Reserved.


#include "ExperienceWorldSettings.h"

#include "GameplayExperiencesLog.h"
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceWorldSettings)

AExperienceWorldSettings::AExperienceWorldSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FPrimaryAssetId AExperienceWorldSettings::GetDefaultGameplayExperience() const
{
	FPrimaryAssetId Result;
	if (!DefaultGameplayExperience.IsNull())
	{
		Result = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameplayExperience.ToSoftObjectPath());

		if (!Result.IsValid())
		{
			EXPERIENCE_LOG(Error, TEXT("%s.DefaultExperience is %s but that failed to resolve into an asset ID."),
				*GetPathNameSafe(this), *DefaultGameplayExperience.ToString());
		}
	}
	else
	{
		EXPERIENCE_LOG(Verbose, TEXT("%s.DefaultExperience is not set."), *GetPathNameSafe(this));
	}

	EXPERIENCE_LOG(Display, TEXT("Retrieving default experience %s from world settings."), *Result.ToString());

	return Result;
}
