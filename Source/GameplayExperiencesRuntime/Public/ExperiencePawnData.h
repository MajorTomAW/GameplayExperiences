// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "ExperiencePawnData.generated.h"

class UInputConfig;
/**
 * Experience data asset that contains properties used to define a pawn for the experience. 
 */
UCLASS(BlueprintType, Const, meta = (DisplayName = "Pawn Data", ShortTooltip = "Data asset used to define a Pawn."), HideDropdown)
class GAMEPLAYEXPERIENCESRUNTIME_API UExperiencePawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UExperiencePawnData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	/** Class to instantiate for this pawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
	TSubclassOf<APawn> PawnClass;

	/** Input config to use for player-controlled pawns. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
	TSoftObjectPtr<UInputConfig> InputConfig;
};
