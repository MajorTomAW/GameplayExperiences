// Copyright © 2024 Playton. All Rights Reserved.


#include "Components/ExperienceManagerComponent.h"

#include "ExperienceDefinition.h"
#include "ExperienceManagerSubsystem.h"
#include "GameFeatureAction.h"
#include "GameFeatureActionSet.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesSubsystemSettings.h"
#include "GameplayExperiencesLog.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceManagerComponent)

//@TODO: Async load the experience definition itself
//@TODO: Handle failures explicitly (go into a 'completed but failed' state rather than check()-ing)
//@TODO: Do the action phases at the appropriate times instead of all at once
//@TODO: Support deactivating an experience and do the unloading actions
//@TODO: Think about what deactivation/cleanup means for preloaded assets
//@TODO: Handle deactivating game features, right now we 'leak' them enabled
// (for a client moving from experience to experience we actually want to diff the requirements and only unload some, not unload everything for them to just be immediately reloaded)
//@TODO: Handle both built-in and URL-based plugins (search for colon?)

UExperienceManagerComponent::UExperienceManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

UExperienceManagerComponent* UExperienceManagerComponent::Get(const UObject* WorldContextObject)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (World->GetGameState())
		{
			return Get(World->GetGameState());
		}
	}

	return nullptr;
}

UExperienceManagerComponent* UExperienceManagerComponent::Get(const AGameStateBase* GameState)
{
	return GameState->FindComponentByClass<UExperienceManagerComponent>();
}

void UExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Deactivate any features this experience loaded
	for (const FString& PluginRUL : GameFeaturePluginURLs)
	{
		if (UExperienceManagerSubsystem::RequestToDeactivatePlugin(PluginRUL))
		{
			UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginRUL);
		}
	}

	if (LoadState == EExperienceLoadState::Loaded)
	{
		LoadState = EExperienceLoadState::Deactivating;

		// Make sure we won't complete the transition prematurely if someone registers as a pauser but fires immediately
		NumExpectedPausers = INDEX_NONE;
		NumObservedPausers = 0;

		// Deactivate and unload the actions
		FGameFeatureDeactivatingContext Context(TEXT(""), [this](FStringView) { this->OnActionDeactivationCompleted(); });

		const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
		if (WorldContext)
		{
			Context.SetRequiredWorldContextHandle(WorldContext->ContextHandle);
		}

		auto DeactivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
		{
			for (UGameFeatureAction* Action : ActionList)
			{
				if (Action)
				{
					Action->OnGameFeatureDeactivating(Context);
					Action->OnGameFeatureUnregistering();
				}
			}
		};

		DeactivateListOfActions(CurrentExperience->FeatureActions);
		for (const TObjectPtr<UGameFeatureActionSet>& ActionSet : CurrentExperience->FeatureActionSets)
		{
			if (ActionSet != nullptr)
			{
				DeactivateListOfActions(ActionSet->Actions);
			}
		}

		NumExpectedPausers = Context.GetNumPausers();

		if (NumExpectedPausers > 0)
		{
			EXPERIENCE_LOG(Error, TEXT("Actions that have asynchronous deactivation aren't fully supported yet."));
		}

		if (NumExpectedPausers == NumObservedPausers)
		{
			OnAllActionsDeactivated();
		}
	}
}

void UExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentExperience);
}

void UExperienceManagerComponent::SetCurrentExperience(FPrimaryAssetId ExperienceId)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceId);
	TSubclassOf<UExperienceDefinition> AssetClass = Cast<UClass>(AssetPath.TryLoad());
	check(AssetClass);

	const UExperienceDefinition* Experience = GetDefault<UExperienceDefinition>(AssetClass);

	check(Experience != nullptr);
	check(CurrentExperience == nullptr);
	CurrentExperience = Experience;
	StartExperienceLoad();
}

bool UExperienceManagerComponent::IsExperienceLoaded() const
{
	return (LoadState == EExperienceLoadState::Loaded) && (CurrentExperience != nullptr);
}

const UExperienceDefinition* UExperienceManagerComponent::GetLoadedExperience() const
{
	check(LoadState == EExperienceLoadState::Loaded);
	return CurrentExperience;
}

const UExperienceDefinition* UExperienceManagerComponent::GetLoadedExperience_Checked() const
{
	check(LoadState == EExperienceLoadState::Loaded);
	check(CurrentExperience);
	return CurrentExperience;
}

void UExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_HighPriority(
	FOnExperienceLoaed::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_HighPriority.Add(MoveTemp(Delegate));
	}
}

void UExperienceManagerComponent::CallOrRegister_OnExperienceLoaded(
	FOnExperienceLoaed::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded.Add(MoveTemp(Delegate));
	}
}

void UExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_LowPriority(
	FOnExperienceLoaed::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_LowPriority.Add(MoveTemp(Delegate));
	}
}

void UExperienceManagerComponent::OnRep_CurrentExperience()
{
	StartExperienceLoad();
}

void UExperienceManagerComponent::StartExperienceLoad()
{
	check(CurrentExperience != nullptr);
	check(LoadState == EExperienceLoadState::Unloaded);

	EXPERIENCE_LOG(Log, TEXT("Starting experience load for '%s' (%s)"),
		*CurrentExperience->GetPrimaryAssetId().ToString(),
		*GetClientServerContextString(this));

	LoadState = EExperienceLoadState::Loading;

	UAssetManager& AssetManager = UAssetManager::Get();
	TSet<FPrimaryAssetId> BundleAssetList;
	TSet<FSoftObjectPath> RawAssetList;

	BundleAssetList.Add(CurrentExperience->GetPrimaryAssetId());
	for (const TObjectPtr<UGameFeatureActionSet>& ActionSet : CurrentExperience->FeatureActionSets)
	{
		if (ActionSet != nullptr)
		{
			BundleAssetList.Add(ActionSet->GetPrimaryAssetId());
		}
	}

	// Load assets associated with the experience
	TArray<FName> BundlesToLoad;
	BundlesToLoad.Add("Equipped");

	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	const bool bLoadClient = GIsEditor || (OwnerNetMode != NM_DedicatedServer);
	const bool bLoadServer = GIsEditor || (OwnerNetMode != NM_Client);

	if (bLoadClient)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (bLoadServer)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}

	TSharedPtr<FStreamableHandle> BundleLoadHandle = nullptr;
	if (BundleAssetList.Num() > 0)
	{
		BundleLoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(BundleAssetList.Array(), BundlesToLoad, {}, false, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority);
	}

	TSharedPtr<FStreamableHandle> RawLoadHandle = nullptr;
	if (RawAssetList.Num() > 0)
	{
		RawLoadHandle = AssetManager.LoadAssetList(RawAssetList.Array(), FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority);
	}

	// If both async loads are running, combine them
	TSharedPtr<FStreamableHandle> CombinedHandle = nullptr;
	if (BundleLoadHandle.IsValid() && RawLoadHandle.IsValid())
	{
		CombinedHandle = AssetManager.GetStreamableManager().CreateCombinedHandle({BundleLoadHandle, RawLoadHandle});
	}
	else
	{
		CombinedHandle = BundleLoadHandle.IsValid() ? BundleLoadHandle : RawLoadHandle;
	}

	// Request the async load
	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnExperienceLoadComplete);
	if (!CombinedHandle.IsValid() || CombinedHandle->HasLoadCompleted())
	{
		// Assets were already loaded, call the completion delegate immediately
		FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	}
	else
	{
		CombinedHandle->BindCancelDelegate(OnAssetsLoadedDelegate);
		CombinedHandle->BindCancelDelegate(FStreamableDelegate::CreateLambda([OnAssetsLoadedDelegate]()
		{
			OnAssetsLoadedDelegate.ExecuteIfBound();
		}));
	}

	// Set of assets to preload ?
	TSet<FPrimaryAssetId> PreloadAssetList;
	if (PreloadAssetList.Num() > 0)
	{
		AssetManager.ChangeBundleStateForPrimaryAssets(PreloadAssetList.Array(), BundlesToLoad, {});
	}
}

void UExperienceManagerComponent::OnExperienceLoadComplete()
{
	check(LoadState == EExperienceLoadState::Loading);
	check(CurrentExperience != nullptr);

	EXPERIENCE_LOG(Log, TEXT("Experience '%s' (%s) load completed!"),
		*CurrentExperience->GetPrimaryAssetId().ToString(),
		*GetClientServerContextString(this));

	// Find the URLs for our GameFeaturePlugins
	// Filtering out dupes and invalid mappings
	GameFeaturePluginURLs.Reset();
	auto CollectGameFeaturePluginURLs = [This=this](const UPrimaryDataAsset* Context, const TArray<FString>& FeaturePluginList)
	{
		for (const FString& PluginName : FeaturePluginList)
		{
			FString PluginURL;
			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName, PluginURL))
			{
				This->GameFeaturePluginURLs.AddUnique(PluginURL);
			}
			else
			{
				ensureMsgf(false, TEXT("OnExperienceLoadComplete: Failed to find URL for GameFeaturePlugin '%s' for experience '%s'"),
					*PluginName,
					*Context->GetPrimaryAssetId().ToString());
			}
		}
	};

	// Collect the URLs for the GameFeaturePlugins
	TArray<FString> FeaturePluginList;
	for (const auto& Plugin : CurrentExperience->GameFeaturesToEnable)
	{
		FeaturePluginList.Add(Plugin.GetPluginName());
	}
	CollectGameFeaturePluginURLs(CurrentExperience, FeaturePluginList);

	// Load and activate the features
	if (NumGameFeaturePluginsLoading > 0)
	{
		LoadState = EExperienceLoadState::LoadingGameFeatures;
		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			UExperienceManagerSubsystem::NotifyOfPluginActivation(PluginURL);
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(PluginURL, FGameFeaturePluginLoadComplete::CreateUObject(this, &ThisClass::OnGameFeaturePluginLoadComplete));
		}
	}
	else
	{
		OnExperienceFullLoadCompleted();
	}
}

void UExperienceManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	// Decrement the number of plugins loading
	NumGameFeaturePluginsLoading--;

	if (NumGameFeaturePluginsLoading == 0)
	{
		OnExperienceFullLoadCompleted();
	}
}

void UExperienceManagerComponent::OnActionDeactivationCompleted()
{
	check(IsInGameThread());
	++NumObservedPausers;

	if (NumExpectedPausers == NumObservedPausers)
	{
		OnAllActionsDeactivated();
	}
}

void UExperienceManagerComponent::OnAllActionsDeactivated()
{
	LoadState = EExperienceLoadState::Unloaded;
	CurrentExperience = nullptr;
}

void UExperienceManagerComponent::OnExperienceFullLoadCompleted()
{
	check(LoadState != EExperienceLoadState::Loaded);

	// Random delay for chaos testing ?

	// Execute the actions
	LoadState = EExperienceLoadState::ExecutingActions;
	FGameFeatureActivatingContext Context;

	const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	if (WorldContext)
	{
		Context.SetRequiredWorldContextHandle(WorldContext->ContextHandle);
	}

	auto ActivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
	{
		for (UGameFeatureAction* Action : ActionList)
		{
			if (Action != nullptr)
			{
				// Might be problematic in a client-server PIE due to them not taking a world.
				// Applying results to actors is restricted to a specific world.
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}
	};

	ActivateListOfActions(CurrentExperience->FeatureActions);
	for (const TObjectPtr<UGameFeatureActionSet>& ActionSet : CurrentExperience->FeatureActionSets)
	{
		if (ActionSet != nullptr)
		{
			ActivateListOfActions(ActionSet->Actions);
		}
	}

	LoadState = EExperienceLoadState::Loaded;

	OnExperienceLoaded_HighPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_HighPriority.Clear();

	OnExperienceLoaded.Broadcast(CurrentExperience);
	OnExperienceLoaded.Clear();

	OnExperienceLoaded_LowPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_LowPriority.Clear();
}
