// Copyright © 2024 Playton. All Rights Reserved.


#include "GameFramework/ExperiencePlayerState.h"

#include "GameplayExperiencesLog.h"
#include "ModularExperienceGameMode.h"
#include "ExperienceDefinition.h"
#include "Components/ExperienceManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperiencePlayerState)

AExperiencePlayerState::AExperiencePlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AExperiencePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, Params);
}

void AExperiencePlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* World = GetWorld();
	if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
	{
		const AGameStateBase* GameState = World->GetGameState();
		check(GameState);

		UExperienceManagerComponent* ExperienceMgr = GameState->FindComponentByClass<UExperienceManagerComponent>();
		check(ExperienceMgr);

		ExperienceMgr->CallOrRegister_OnExperienceLoaded(FOnExperienceLoaed::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

void AExperiencePlayerState::SetPawnData(const UExperiencePawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		EXPERIENCE_LOG(Error, TEXT("Attempting to set PawnData [%s] on player state [%s] that already has a valid PawnData [%s]"),
			*GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	const UExperiencePawnData* OldPawnData = PawnData;
	PawnData = InPawnData;

	OnPawnDataChanged(OldPawnData, PawnData);

	ForceNetUpdate();
}

void AExperiencePlayerState::OnExperienceLoaded(const UExperienceDefinition* CurrentExperience)
{
	if (const AModularExperienceGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AModularExperienceGameModeBase>())
	{
		if (const UExperiencePawnData* NewPawnData = GameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			EXPERIENCE_LOG(Error, TEXT("OnExperienceLoaded() called but no pawn data found for player state %s"), *GetNameSafe(this));
		}
	}
}

void AExperiencePlayerState::OnRep_PawnData()
{
}
