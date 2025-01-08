// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction_AddInputMappingContext.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"

#include "ExperienceHeroComponent.generated.h"


class UInputConfig;

namespace EEndPlayReason { enum Type : int; }
struct FLoadedMappableConfigPair;
struct FMappableConfigPair;

class UGameFrameworkComponentManager;
class UInputComponent;

struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;
struct FInputActionValue;

/**
 * Basic hero component for player-controlled pawns that sets up input and camera handling.
 * Depends on the UExperiencePawnExtensionComponent to coordinate initialization.
 */
UCLASS(ClassGroup=(GameplayExperiences), meta=(BlueprintSpawnableComponent))
class GAMEPLAYEXPERIENCESRUNTIME_API UExperienceHeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UExperienceHeroComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Static getter that returns the hero component if one exists on the specified actor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GameplayExperiences)
	static UExperienceHeroComponent* FindHeroComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UExperienceHeroComponent>() : nullptr); }

	/** The name of this overall feature, depends on the other named component features. */
	static const FName NAME_ActorFeatureName;

	/** True if this is controlled by a real player and has progressed far enough in initialization where additional input bindings can be added. */
	bool IsReadyToBindInputs() const { return bReadyToBindInputs; }

	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

	/** Initializes the player input with the given input component. */
	void InitializePlayerInput(UInputComponent* InputComponent);
	virtual void OnInitializePlayerInput(UInputComponent* InputComponent, const UInputConfig* InputConfig) {}

protected:
	//~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRegister() override;
	//~ End UActorComponent interface

protected:
	/** True, when the player input bindings have been applied, will never be true for non-player controlled pawns. */
	uint8 bReadyToBindInputs : 1;

	/** List of default input mappings to give to the input component. */
	UPROPERTY(EditAnywhere, Category = "Hero|Input")
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
};
