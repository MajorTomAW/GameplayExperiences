// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "ExperienceManagerComponent.generated.h"

class UExperienceDefinition;

namespace UE::GameFeatures
{
	struct FResult;
}

DECLARE_MULTICAST_DELEGATE_OneParam(FOnExperienceLoaed, const UExperienceDefinition* /*Experience*/);

enum class EExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating,
};

/**
 * Manager component that manages the loading and unloading of gameplay experience.
 */
UCLASS(Config = Game)
class GAMEPLAYEXPERIENCESRUNTIME_API UExperienceManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	static UExperienceManagerComponent* Get(const UObject* WorldContextObject);
	static UExperienceManagerComponent* Get(const AGameStateBase* GameState);

	//~ Begin UActorComponent Interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End UActorComponent Interface

	/** Returns true if the experience has been fully loaded. */
	bool IsExperienceLoaded() const;

	/** Returns the currently loaded experience. */
	const UExperienceDefinition* GetLoadedExperience() const;

	/** Returns the current experience if it has been loaded, otherwise asserts. */
	const UExperienceDefinition* GetLoadedExperience_Checked() const;

	/** Tries to set the current experience. */
	void SetCurrentExperience(FPrimaryAssetId ExperienceId);

	/**
	 * Ensures the delegate is called once the experience has been loaded, before others are called.
	 * However, if the experience has already loaded, the delegate is called immediately.
	 */
	void CallOrRegister_OnExperienceLoaded_HighPriority(FOnExperienceLoaed::FDelegate&& Delegate);

	/**
	 * Ensures the delegate is called once the experience has been loaded.
	 * If the experience has already loaded, the delegate is called immediately.
	 */
	void CallOrRegister_OnExperienceLoaded(FOnExperienceLoaed::FDelegate&& Delegate);

	/**
	 * Ensures the delegate is called once the experience has been loaded, after others are called.
	 * If the experience has already loaded, the delegate is called immediately.
	 */
	void CallOrRegister_OnExperienceLoaded_LowPriority(FOnExperienceLoaed::FDelegate&& Delegate);

protected:
	UFUNCTION()
	virtual void OnRep_CurrentExperience();

	virtual void StartExperienceLoad();
	void OnExperienceLoadComplete();
	void OnExperienceFullLoadCompleted();
	
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);

	void OnActionDeactivationCompleted();
	void OnAllActionsDeactivated();

private:
	/** Replicated experience */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentExperience)
	TObjectPtr<const UExperienceDefinition> CurrentExperience;

	EExperienceLoadState LoadState = EExperienceLoadState::Unloaded;

	int32 NumGameFeaturePluginsLoading = 0;
	TArray<FString> GameFeaturePluginURLs;

	int32 NumObservedPausers = 0;
	int32 NumExpectedPausers = 0;

	FOnExperienceLoaed OnExperienceLoaded_HighPriority;
	FOnExperienceLoaed OnExperienceLoaded;
	FOnExperienceLoaed OnExperienceLoaded_LowPriority;
};
