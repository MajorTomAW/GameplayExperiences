// Copyright © 2024 Playton. All Rights Reserved.


#include "ExperienceAssetManager.h"

#include "GameplayExperiencesLog.h"
#include "Developer/ExperienceGameSettings.h"

#include "ExperiencePawnData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceAssetManager)

UExperienceAssetManager::UExperienceAssetManager()
{
}

UExperienceAssetManager& UExperienceAssetManager::Get()
{
	check(GEngine);

	if (UExperienceAssetManager* Singleton = Cast<UExperienceAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	EXPERIENCE_LOG(Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini, must be UExperienceAssetManager!"));

	// Fatal error above prevents this from ever being hit, but compiler doesn't know that :P
	return *NewObject<UExperienceAssetManager>();
}

const UExperienceGameData* UExperienceAssetManager::K2_GetGameData(const TSubclassOf<UExperienceGameData> GameDataClass)
{
	return &Get().GetGameData();
}

const UExperienceGameData& UExperienceAssetManager::GetGameData()
{
	return GetOrLoadTypedGameData<UExperienceGameData>(TSoftObjectPtr<UExperienceGameData>(UExperienceGameSettings::Get()->GameDataPath));
}

const UExperiencePawnData* UExperienceAssetManager::GetDefaultPawnData() const
{
	FSoftObjectPath PawnDataPath = UExperienceGameSettings::Get()->DefaultPawnData;
	return GetAsset<UExperiencePawnData>(TSoftObjectPtr<UExperiencePawnData>(PawnDataPath));
}

UObject* UExperienceAssetManager::SynchronousLoadAsset(const FSoftObjectPath& Path)
{
	if (Path.IsValid())
	{
		TUniquePtr<FScopeLogTime> ScopedLogTime;

		if (ShouldLogAssetLoads())
		{
			ScopedLogTime = MakeUnique<FScopeLogTime>(*FString::Printf(TEXT("Synchronously loaded asset [%s]"), *Path.ToString()), nullptr, FScopeLogTime::ScopeLog_Seconds);
		}

		if (UAssetManager::IsInitialized())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(Path, false);
		}

		// Use LoadObject if the asset manager isn't initialized yet
		return Path.TryLoad();
	}

	return nullptr;
}

bool UExperienceAssetManager::ShouldLogAssetLoads()
{
	static bool bLogAssetLoads = FParse::Param(FCommandLine::Get(), TEXT("LogAssetLoads"));
	return bLogAssetLoads;
}

void UExperienceAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);
		LoadedAssets.Add(Asset);
	}
}

UPrimaryDataAsset* UExperienceAssetManager::LoadGameDataOfClass(
	TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& Path, FPrimaryAssetType AssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GameData Object"), STAT_GameData, STATGROUP_LoadTime);
	if (!Path.IsNull())
	{
#if WITH_EDITOR
		FScopedSlowTask SlowTask(0, FText::Format(NSLOCTEXT("BotaniEditor", "LoadingGameDataTask", "Loading GameData {0}"), FText::FromName(DataClass->GetFName())));
		constexpr bool bShowCancelButton = false;
		constexpr bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
#endif
		EXPERIENCE_LOG(Log, TEXT("Loading GameData %s"), *Path.ToString());
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("		... GameData loaded!"), nullptr);

		// This can be called recursively in the editor because it is called on demand from PostLoad so force a sync load for primary asset and async load the rest in that case
		if (GIsEditor)
		{
			Asset = Path.LoadSynchronous();
			LoadPrimaryAssetsWithType(AssetType);
		}
		else
		{
			TSharedPtr<FStreamableHandle> Streamable = LoadPrimaryAssetsWithType(AssetType);
			if (Streamable.IsValid())
			{
				Streamable->WaitUntilComplete(0.f, false);
				Asset = Cast<UPrimaryDataAsset>(Streamable->GetLoadedAsset());
			}
		}
	}

	if (Asset)
	{
		GameDataMap.Add(DataClass, Asset);
	}
	else
	{
		EXPERIENCE_LOG(Fatal, TEXT("Failed to load GameData asset at %s. Type %s. This is not recoverable and likely means you do not have the correct data to run %s."),
			*Path.ToString(), *AssetType.ToString(), FApp::GetProjectName());
	}

	return Asset;
}
