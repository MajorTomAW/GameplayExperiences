// Copyright © 2024 Playton. All Rights Reserved.


#include "ExperienceManagerSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameplayExperiencesLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceManagerSubsystem)

UExperienceManagerSubsystem::UExperienceManagerSubsystem()
{
}

UExperienceManagerSubsystem* UExperienceManagerSubsystem::Get()
{
	return GEngine->GetEngineSubsystem<UExperienceManagerSubsystem>();
}

#if WITH_EDITOR
void UExperienceManagerSubsystem::OnPlayInEditorBegun()
{
	ensure(GameFeaturePluginRequestCountMap.IsEmpty());
	GameFeaturePluginRequestCountMap.Empty();
}

void UExperienceManagerSubsystem::NotifyOfPluginActivation(const FString PluginURL)
{
	if (GIsEditor)
	{
		UExperienceManagerSubsystem* MutableThis = GEngine->GetEngineSubsystem<UExperienceManagerSubsystem>();
		check(MutableThis);

		// Track the number of requesters who have requested this plugin to be activated
		int32& Count = MutableThis->GameFeaturePluginRequestCountMap.FindOrAdd(PluginURL);
		++Count;

		EXPERIENCE_LOG(Log, TEXT("Request to activate plugin '%s' (new listeners count %d)"), *PluginURL, Count);
	}
}

bool UExperienceManagerSubsystem::RequestToDeactivatePlugin(const FString PluginURl)
{
	if (GIsEditor)
	{
		UExperienceManagerSubsystem* MutableThis = GEngine->GetEngineSubsystem<UExperienceManagerSubsystem>();
		check(MutableThis);

		// Only let the last requester to get this far deactivate the plugin
		int32& Count = MutableThis->GameFeaturePluginRequestCountMap.FindChecked(PluginURl);
		--Count;

		if (Count == 0)
		{
			EXPERIENCE_LOG(Log, TEXT("No more requests for plugin '%s', deactivating."), *PluginURl);
			MutableThis->GameFeaturePluginRequestCountMap.Remove(PluginURl);
			return true;
		}

		return false;
	}

	return true;
}

#endif