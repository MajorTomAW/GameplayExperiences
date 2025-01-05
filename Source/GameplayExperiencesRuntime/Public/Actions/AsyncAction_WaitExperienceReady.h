// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "AsyncAction_WaitExperienceReady.generated.h"

class AGameStateBase;
class UExperienceDefinition;
class UWorld;
struct FFrame;

/** Determines the priority of the experience load. */
UENUM()
enum class EExperienceLoadPriority : uint8
{
	/** The experience load is low priority. */
	Low,

	/** The experience load is normal priority. */
	Normal,

	/** The experience load is high priority. (For most gameplay relevant actions) */
	High
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExperienceReadyAsyncDelegate);

/**
 * Asynchronously waits for the game state to be ready for the experience to be loaded.
 */
UCLASS()
class UAsyncAction_WaitExperienceReady : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UAsyncAction_WaitExperienceReady(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Waits for the experience to be ready and loaded. */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Wait for Experience Ready"), Category = "Experience")
	static UAsyncAction_WaitExperienceReady* WaitForExperienceReady(UObject* WorldContextObject, EExperienceLoadPriority Priority = EExperienceLoadPriority::Normal);

	/** Called when the experience is ready. */
	UPROPERTY(BlueprintAssignable)
	FExperienceReadyAsyncDelegate OnReady;

protected:
	//~ Begin UBlueprintAsyncActionBase Interface
	virtual void Activate() override;
	//~ End UBlueprintAsyncActionBase Interface

private:
	void Step1_HandleGameStateSet(AGameStateBase* GameState);
	void Step2_ListenToExperienceLoading(AGameStateBase* GameState);
	void Step3_HandleExperienceLoaded(const UExperienceDefinition* CurrentExperience);
	void Step4_BroadcastReady();

private:
	TWeakObjectPtr<UWorld> WorldPtr;
	EExperienceLoadPriority Priority;
};
