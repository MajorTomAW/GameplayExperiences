// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "EngineDefines.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameplayExperiences, Log, All);

namespace GameplayExperiences
{
	FString GetClientServerContextString(UObject* Context);
	
#define EXPERIENCE_LOG(Verbosity, Format, ...) \
UE_LOG(LogGameplayExperiences, Verbosity, Format, ##__VA_ARGS__)

#define EXPERIENCE_C_LOG(Verbosity, Format, ...) \
UE_LOG(LogGameplayExperiences, Verbosity, TEXT("%s: ") Format, *FString(__FUNCTION__), ##__VA_ARGS__)

#define EXPERIENCE_NET_LOG(Verbosity, Object, Format, ...) \
UE_LOG(LogGameplayExperiences, Verbosity, TEXT("%s: ") Format, *GetClientServerContextString(Object), ##__VA_ARGS__)
}
