// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Developer/ExperienceGameSettings.h"
#include "ExperienceGameData.h"
#include "Engine/AssetManager.h"

#include "ExperienceAssetManager.generated.h"

class UExperiencePawnData;
class UExperienceGameData;

/**
 * Asset manager for the Experience system.
 * Specifically, responsible for loading and managing Experience assets. (Global Game Data)
 */
UCLASS(Config = Game)
class GAMEPLAYEXPERIENCESRUNTIME_API UExperienceAssetManager : public UAssetManager
{
	GENERATED_BODY()
	friend class UExperienceGameSettings;

public:
	UExperienceAssetManager();
	static UExperienceAssetManager& Get();

	/** Returns the global game data asset */
	UFUNCTION(BlueprintCallable, Category = "Experience", meta = (DisplayName = "Get Game Data", DeterminesOutputType = "GameDataClass"))
	static const UExperienceGameData* K2_GetGameData(const TSubclassOf<UExperienceGameData> GameDataClass);

	/** Returns the global game data asset */
	virtual const UExperienceGameData& GetGameData();
	virtual const UExperiencePawnData* GetDefaultPawnData() const;

	/** Returns the global game data asset typed */
	template <typename GameData>
	const GameData& GetGameDataTyped()
	{
		return (GameData&)GetGameData();
	}

	/** Loads a types game data asset */
	template <typename GameDataClass>
	const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& Path)
	{
		if (TObjectPtr<UPrimaryDataAsset> const* pResult = GameDataMap.Find(GameDataClass::StaticClass()))
		{
			return *CastChecked<GameDataClass>(*pResult);
		}

		// Otherwise perform a blocking load
		return *CastChecked<const GameDataClass>(LoadGameDataOfClass(GameDataClass::StaticClass(), Path, GameDataClass::StaticClass()->GetFName()));
	}

	/** Returns the asset referenced by the soft object pointer. Performs a synchronous load if necessary. */
	template <typename AssetType>
	static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPtr, bool bKeepInMemory = true)
	{
		AssetType* Asset = nullptr;

		const FSoftObjectPath& Path = AssetPtr.ToSoftObjectPath();
		if (Path.IsValid())
		{
			Asset = AssetPtr.Get();
			if (!Asset)
			{
				Asset = Cast<AssetType>(SynchronousLoadAsset(Path));
				ensureAlwaysMsgf(Asset, TEXT("Failed to load asset %s"), *Path.ToString());
			}

			if (Asset && bKeepInMemory)
			{
				Get().AddLoadedAsset(Cast<UObject>(Asset));
			}
		}

		return Asset;
	}

	/** Returns the subclass referenced by the soft class pointer. Performs a synchronous load if necessary. */
	template <typename ClassType>
	static TSubclassOf<ClassType> GetSubclass(const TSoftClassPtr<ClassType>& AssetPtr, bool bKeepInMemory = true)
	{
		TSubclassOf<ClassType> LoadedSubClass;
		const FSoftObjectPath& Path = AssetPtr.ToSoftObjectPath();

		if (Path.IsValid())
		{
			LoadedSubClass = AssetPtr.Get();
			if (LoadedSubClass == nullptr)
			{
				LoadedSubClass = Cast<UClass>(SynchronousLoadAsset(Path));
				ensureAlwaysMsgf(LoadedSubClass, TEXT("Failed to load asset class %s"), *AssetPtr.ToString());
			}

			if (LoadedSubClass && bKeepInMemory)
			{
				Get().AddLoadedAsset(Cast<UObject>(LoadedSubClass));
			}
		}

		return LoadedSubClass;
	}

protected:
	/** Performs a blocking load of the asset. */
	static UObject* SynchronousLoadAsset(const FSoftObjectPath& Path);
	static bool ShouldLogAssetLoads();

	/** Thread safe way of adding a loaded asset to keep in memory. */
	void AddLoadedAsset(const UObject* Asset);
	
	/** Blocking-ly loads the game data asset. */
	UPrimaryDataAsset* LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& Path, FPrimaryAssetType AssetType);

protected:
	/** Loaded version of the game data asset. (Can be multiple assets based on your project's needs) */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TObjectPtr<UPrimaryDataAsset>> GameDataMap;
	
	/** Assets loaded and tracked by the asset manager. */
	UPROPERTY()
	TSet<TObjectPtr<const UObject>> LoadedAssets;

	/** Used for scope lock when modifying the list of loaded assets. */
	FCriticalSection LoadedAssetsCritical;
};
