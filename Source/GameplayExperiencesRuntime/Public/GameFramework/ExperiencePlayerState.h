// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExperiencePawnData.h"
#include "ModularPlayerState.h"
#include "ExperiencePlayerState.generated.h"

class UExperienceDefinition;
class UExperiencePawnData;
/**
 * A player state that is used in the context of a gameplay experience.
 */
UCLASS(Config = Game)
class GAMEPLAYEXPERIENCESRUNTIME_API AExperiencePlayerState : public AModularPlayerState
{
	GENERATED_BODY()

public:
	AExperiencePlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	template <class T>
	const T* GetPawnData() const
	{
		return Cast<T>(PawnData);
	}
	const UExperiencePawnData* GetPawnData() const
	{
		return PawnData;
	}

	/** Sets the pawn data for this player state. */
	void SetPawnData(const UExperiencePawnData* InPawnData);

	//~ Begin APlayerState Interface
	virtual void PostInitializeComponents() override;
	//~ End APlayerState Interface

protected:
	/** The pawn data that gets set by the game mode / experience. */
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UExperiencePawnData> PawnData;

protected:
	virtual void OnPawnDataChanged(const UExperiencePawnData* OldPawnData, const UExperiencePawnData* NewPawnData) {}
	virtual void OnExperienceLoaded(const UExperienceDefinition* CurrentExperience);
	
	UFUNCTION()
	virtual void OnRep_PawnData();
};
