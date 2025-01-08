// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExperiencePawnData.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"

#include "ExperiencePawnExtensionComponent.generated.h"

class UExperiencePawnData;

namespace EEndPlayReason { enum Type : int; }

class UAbilitySystemComponent;

/**
 * Centralized component that adds functionality to all pawn classes so it can be used for characters/vehicles/etc.
 * This coordinates initialization and cleanup of other components.
 */
UCLASS(ClassGroup = (GameplayExperiences))
class GAMEPLAYEXPERIENCESRUNTIME_API UExperiencePawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UExperiencePawnExtensionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Static getter that returns the pawn extension component if one exists on the specified actor. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GameplayExperiences)
	static UExperiencePawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UExperiencePawnExtensionComponent>() : nullptr); }

	/** The name of this overall feature, depends on the other named component features. */
	static const FName NAME_ActorFeatureName;

	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

	//~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRegister() override;
	//~ End UActorComponent interface

	/** Should be called by the owning pawn to become the avatar of the ability system. */
	virtual void InitializeAbilitySystem(UAbilitySystemComponent* InASC, AActor* InOwnerActor);

	/** Should be called by the owning pawn to remove itself as the avatar of the ability system. */
	virtual void UninitializeAbilitySystem();

	/** Should be called by the owning pawn when the pawn's controller changes. */
	virtual void HandleControllerChanged();

	/** Should be called by the owning pawn when the player state has been replicated. */
	virtual void HandlePlayerStateReplicated();

	/** Should be called by the owning pawn when the input component is setup. */
	virtual void SetupPlayerInputComponent();

	/** Registers with the OnAbilitySystemInitialized delegate. Directly calls the delegate if the ability system is already initialized. */
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	/** Registers with the OnAbilitySystemUninitialized delegate. */
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

	/** Returns the ability system component. */
	template <class T = UAbilitySystemComponent>
	T* GetAbilitySystemComponent() const { return Cast<T>(AbilitySystem); }

	/** Returns the ability system component. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GameplayExperiences)
	UAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystem; }
	
	/** Returns the typed pawn data. */
	template <class T = UExperiencePawnData>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	/** Returns the pawn data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GameplayExperiences)
	const UExperiencePawnData* GetPawnData() const { return PawnData; }

	/** Sets the current pawn data. */
	UFUNCTION(BlueprintCallable, Category = GameplayExperiences)
	void SetPawnData(const UExperiencePawnData* InPawnData);

protected:
	/** OnRep function for PawnData. */
	UFUNCTION()
	virtual void OnRep_PawnData();

	/** Delegate fired when our pawn becomes the ability system's avatar. */
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	/** Delegate fired when our pawn's ability system is uninitialized. */
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

protected:
	/** Pawn data used to create the pawn. Specified from a spawn function or on a placed instance. */
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = Pawn)
	TObjectPtr<const UExperiencePawnData> PawnData;
	
	/** Pointer to the ability system component. Cached for quick access. */
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> AbilitySystem;
};
