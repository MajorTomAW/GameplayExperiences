// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeaturePluginURL.h"
#include "Engine/DataAsset.h"

#include "ExperienceDefinition.generated.h"

class UExperiencePawnData;
class UGameFeatureActionSet;
class UGameFeatureAction;
struct FPrimaryAssetTypeInfo;
struct FAssetData;

/**
 * Defines a gameplay experience, a collection of code and content that adds a separable discrete feature to the game.
 */
UCLASS(BlueprintType, Const)
class GAMEPLAYEXPERIENCESRUNTIME_API UExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UExperienceDefinition();

	//~ Begin UPrimaryDataAsset Interface
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
	//~ End UPrimaryDataAsset Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

	/** Returns the list of Game Feature Plugins this experience depends on */
	virtual TArray<FName> GetGameFeaturePluginDependencies() const;

public:
	/** List of Game Feature Plugins this experience depends on */
	UPROPERTY(EditDefaultsOnly, Category = "Dependencies")
	TArray<FGameFeaturePluginURL> GameFeaturesToEnable;

	/** List of individual Game Feature Actions to perform as this experience is loaded/activated/deactivated/unloaded */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Actions")
	TArray<TObjectPtr<UGameFeatureAction>> FeatureActions;

	/** List of additional Game Feature Action Sets to compose into this experience */
	UPROPERTY(EditDefaultsOnly, Category = "Actions")
	TArray<TObjectPtr<UGameFeatureActionSet>> FeatureActionSets;

	/** The default pawn data used by this experience */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TObjectPtr<const UExperiencePawnData> DefaultPawnData;
};
