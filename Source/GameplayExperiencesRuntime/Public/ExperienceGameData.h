// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "ExperienceGameData.generated.h"

/**
 * Non- mutable data asset that contains global game data.
 */
UCLASS(BlueprintType, Const, meta = (DisplayName = "Game Data", ShortTooltip = "Data asset that contains global game data."))
class GAMEPLAYEXPERIENCESRUNTIME_API UExperienceGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UExperienceGameData();
	static const UExperienceGameData& Get();
};
