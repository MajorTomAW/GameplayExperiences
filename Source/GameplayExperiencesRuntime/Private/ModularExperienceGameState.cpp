// Copyright © 2024 Playton. All Rights Reserved.


#include "ModularExperienceGameState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularExperienceGameState)

AModularExperienceGameState::AModularExperienceGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	ExperienceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UExperienceManagerComponent>(this, "ExperienceManagerComponent");
}
