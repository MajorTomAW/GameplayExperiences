// Copyright © 2024 Playton. All Rights Reserved.


#include "Actions/AsyncAction_WaitExperienceReady.h"

#include "Components/ExperienceManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_WaitExperienceReady)

UAsyncAction_WaitExperienceReady::UAsyncAction_WaitExperienceReady(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WorldPtr = nullptr;
	Priority = EExperienceLoadPriority::Normal;
}

UAsyncAction_WaitExperienceReady* UAsyncAction_WaitExperienceReady::WaitForExperienceReady(UObject* WorldContextObject, EExperienceLoadPriority Priority)
{
	UAsyncAction_WaitExperienceReady* Action = nullptr;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		Action = NewObject<UAsyncAction_WaitExperienceReady>();
		Action->WorldPtr = World;
		Action->Priority = Priority;
		Action->RegisterWithGameInstance(World);
	}
	return Action;
}

void UAsyncAction_WaitExperienceReady::Activate()
{
	if (UWorld* World = WorldPtr.Get())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			Step2_ListenToExperienceLoading(GameState);
		}
		else
		{
			World->GameStateSetEvent.AddUObject(this, &ThisClass::Step1_HandleGameStateSet);
		}
	}
	else
	{
		SetReadyToDestroy();
	}
}

void UAsyncAction_WaitExperienceReady::Step1_HandleGameStateSet(AGameStateBase* GameState)
{
	if (UWorld* World = WorldPtr.Get())
	{
		World->GameStateSetEvent.RemoveAll(this);
	}

	Step2_ListenToExperienceLoading(GameState);
}

void UAsyncAction_WaitExperienceReady::Step2_ListenToExperienceLoading(AGameStateBase* GameState)
{
	check(GameState);

	UExperienceManagerComponent* ExperienceMgr = UExperienceManagerComponent::Get(GameState);
	check(ExperienceMgr);

	if (ExperienceMgr->IsExperienceLoaded())
	{
		UWorld* World = GameState->GetWorld();
		check(World);

		// Broadcast ready on the next tick to avoid stack overflow
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::Step4_BroadcastReady));
	}
	else
	{
		FOnExperienceLoaed::FDelegate OnExperienceLoaded = FOnExperienceLoaed::FDelegate::CreateUObject(this, &ThisClass::Step3_HandleExperienceLoaded);

		if (Priority == EExperienceLoadPriority::Normal)
		{
			ExperienceMgr->CallOrRegister_OnExperienceLoaded(MoveTemp(OnExperienceLoaded));
		}
		else if (Priority == EExperienceLoadPriority::Low)
		{
			ExperienceMgr->CallOrRegister_OnExperienceLoaded_LowPriority(MoveTemp(OnExperienceLoaded));
		}
		else if (Priority == EExperienceLoadPriority::High)
		{
			ExperienceMgr->CallOrRegister_OnExperienceLoaded_HighPriority(MoveTemp(OnExperienceLoaded));
		}
	}
}

void UAsyncAction_WaitExperienceReady::Step3_HandleExperienceLoaded(const UExperienceDefinition* CurrentExperience)
{
	Step4_BroadcastReady();
}

void UAsyncAction_WaitExperienceReady::Step4_BroadcastReady()
{
	OnReady.Broadcast();
	SetReadyToDestroy();
}
