// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameState.h"
#include "Components/ExperienceManagerComponent.h"
#include "ModularExperienceGameState.generated.h"

/**
 * Game state for a modular experience. 
 */
UCLASS(Config = Game)
class GAMEPLAYEXPERIENCESRUNTIME_API AModularExperienceGameState : public AModularGameStateBase
{
	GENERATED_BODY()

public:
	AModularExperienceGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** Handles loading and managing the current experience. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UExperienceManagerComponent> ExperienceManagerComponent;
};
