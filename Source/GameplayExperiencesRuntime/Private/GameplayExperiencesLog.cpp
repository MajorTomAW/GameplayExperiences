// Copyright © 2024 Playton. All Rights Reserved.


#include "GameplayExperiencesLog.h"

DEFINE_LOG_CATEGORY(LogGameplayExperiences);

#if WITH_EDITOR
extern ENGINE_API FString GPlayInEditorContextString;
#endif

namespace GameplayExperiences
{
	FString GetClientServerContextString(UObject* Context)
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
			return GPlayInEditorContextString;
		}
#endif

		return TEXT("[]");
	}	
}

#if WITH_EDITOR
extern ENGINE_API FString GPlayInEditorContextString;
#endif

namespace GameplayExperiences
{
	FString GetClientServerContextString(UObject* Context)
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
			return GPlayInEditorContextString;
		}
#endif

		return TEXT("[]");
	}	
}

#if WITH_EDITOR
extern ENGINE_API FString GPlayInEditorContextString;
#endif

namespace GameplayExperiences
{
	FString GetClientServerContextString(UObject* Context)
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
			return GPlayInEditorContextString;
		}
#endif

		return TEXT("[]");
	}	
}
