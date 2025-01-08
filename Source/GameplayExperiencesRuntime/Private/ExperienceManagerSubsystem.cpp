// Copyright © 2024 Playton. All Rights Reserved.


#include "ExperienceManagerSubsystem.h"

#include "GameplayExperiencesLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceManagerSubsystem)

#if WITH_EDITOR

UExperienceManagerSubsystem::UExperienceManagerSubsystem()
{
	ensure(StateChain.Num() == 4);
}

UExperienceManagerSubsystem* UExperienceManagerSubsystem::Get()
{
	return GEngine->GetEngineSubsystem<UExperienceManagerSubsystem>();
}

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