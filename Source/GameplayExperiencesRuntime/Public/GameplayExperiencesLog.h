// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "EngineDefines.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameplayExperiences, Log, All);

#if PLATFORM_DESKTOP
#define EXPERIENCE_LOG(Verbosity, Format, ...) UE_LOG(LogGameplayExperiences, Verbosity, Format, ##__VA_ARGS__)
#endif

inline FString GetClientServerContextString(UObject* Context)
{
	ENetRole Role = ROLE_None;

	if (AActor* Actor = Cast<AActor>(Context))
	{
		Role = Actor->GetLocalRole();
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(Context))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return Role == ROLE_Authority ? TEXT("Server") : TEXT("Client");
	}
	
#if WITH_EDITOR
	if (GIsEditor)
	{
		extern ENGINE_API FString GPlayInEditorContextString;
		return GPlayInEditorContextString;
	}
#endif

	return TEXT("[]");
}
