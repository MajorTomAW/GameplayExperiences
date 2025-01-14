// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Subsystems/EngineSubsystem.h"

#include "ExperienceManagerSubsystem.generated.h"

/**
 * Manager for experiences
 * Primarily for arbitration between multiple PIE sessions
 */
UCLASS(Config = Game)
class GAMEPLAYEXPERIENCESRUNTIME_API UExperienceManagerSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UExperienceManagerSubsystem();
	static UExperienceManagerSubsystem* Get();
	
#if WITH_EDITOR
	void OnPlayInEditorBegun();

	static void NotifyOfPluginActivation(const FString PluginURL);
	static bool RequestToDeactivatePlugin(const FString PluginURl);
#else
	static void NotifyOfPluginActivation(const FString PluginURL) {}
	static bool RequestToDeactivatePlugin(const FString PluginURl) { return true; }
#endif

	FGameplayTag GetTag_Spawned() const { return StateChain[0]; }
	FGameplayTag GetTag_Available() const { return StateChain[1]; }
	FGameplayTag GetTag_Initialized() const { return StateChain[2]; }
	FGameplayTag GetTag_Ready() const { return StateChain[3]; }

public:
	UPROPERTY(Config)
	TArray<FGameplayTag> StateChain;

private:
	/** Map of requests to active count for a given game feature plugin. (allow first in, last out management during PIE) */
	TMap<FString, int32> GameFeaturePluginRequestCountMap;
};
