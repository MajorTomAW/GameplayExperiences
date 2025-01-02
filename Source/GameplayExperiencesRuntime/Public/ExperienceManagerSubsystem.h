// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "Subsystems/EngineSubsystem.h"

#include "ExperienceManagerSubsystem.generated.h"

/**
 * Manager for experiences
 * Primarily for arbitration between multiple PIE sessions
 */
UCLASS(MinimalAPI)
class UExperienceManagerSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	GAMEPLAYEXPERIENCESRUNTIME_API void OnPlayInEditorBegun();

	static void NotifyOfPluginActivation(const FString PluginURL);
	static bool RequestToDeactivatePlugin(const FString PluginURl);
#else
	static void NotifyOfPluginActivation(const FString PluginURL) {}
	static bool RequestToDeactivatePlugin(const FString PluginURl) { return true; }
#endif

private:
	/** Map of requests to active count for a given game feature plugin. (allow first in, last out management during PIE) */
	TMap<FString, int32> GameFeaturePluginRequestCountMap;
};
