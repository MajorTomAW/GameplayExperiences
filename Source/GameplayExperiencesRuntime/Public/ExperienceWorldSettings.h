// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "GameFramework/WorldSettings.h"

#include "ExperienceWorldSettings.generated.h"

class UExperienceDefinition;
/**
 * World settings for a gameplay experience. 
 */
UCLASS()
class GAMEPLAYEXPERIENCESRUNTIME_API AExperienceWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	AExperienceWorldSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	/** Returns the default experience assigned to this world. */
	FPrimaryAssetId GetDefaultGameplayExperience() const;

protected:
	/** The default experience to use in this world. */
	UPROPERTY(EditDefaultsOnly, Category = GameMode)
	TSoftClassPtr<UExperienceDefinition> DefaultGameplayExperience;
};
