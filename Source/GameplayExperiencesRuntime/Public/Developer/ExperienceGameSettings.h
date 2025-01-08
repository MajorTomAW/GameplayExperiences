// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "ExperienceGameSettings.generated.h"

class UExperienceGameData;
/**
 * Developer settings for the gameplay experiences framework.
 */
UCLASS(Config = Game, Defaultconfig, meta = (DisplayName = "Experience Settings"))
class GAMEPLAYEXPERIENCESRUNTIME_API UExperienceGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UExperienceGameSettings();
	static UExperienceGameSettings* Get();

	//~ Begin UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;
	virtual FText GetSectionText() const override;
	virtual void OverrideConfigSection(FString& InOutSectionName) override;
	//~ End UDeveloperSettings Interface

protected:
#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

private:
	void ApplySettings();
#endif

public:
	/** The default experience to use when no experience is specified. */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Defaults", meta = (AllowedTypes="ExperienceDefinition", ConfigRestartRequired = true))
	FPrimaryAssetId DefaultExperience;

	/** The default game data to load for this project. */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Defaults", meta = (MetaClass="/Script/GameplayExperiencesRuntime.ExperienceGameData", ConfigRestartRequired = true))
	FSoftObjectPath GameDataPath;

	/** The default pawn data to use if no pawn data is specified by the current experience. */
	UPROPERTY(Config, EditDefaultsOnly, Category = "Defaults", meta = (MetaClass="/Script/GameplayExperiencesRuntime.ExperiencePawnData", ConfigRestartRequired = true))
	FSoftObjectPath DefaultPawnData;

protected:
	/** The initialization state chain to use for the modular gameplay. */
	UPROPERTY(Config, EditDefaultsOnly, Category = ModularGameplay, meta = (ConfigRestartRequired = true, Categories = "InitState", DisplayName = "State Chain (Order Matters!!!)", EditFixedOrder), EditFixedSize)
	TArray<FGameplayTag> StateChain;
};
