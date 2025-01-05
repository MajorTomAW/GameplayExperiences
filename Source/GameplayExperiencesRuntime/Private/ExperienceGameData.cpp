// Copyright © 2024 Playton. All Rights Reserved.


#include "ExperienceGameData.h"

#include "ExperienceAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceGameData)

UExperienceGameData::UExperienceGameData()
{
}

const UExperienceGameData& UExperienceGameData::Get()
{
	return UExperienceAssetManager::Get().GetGameData();
}
