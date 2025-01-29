// Copyright © 2024 Playton. All Rights Reserved.


#include "Components/ExperiencePawnExtensionComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ExperienceManagerSubsystem.h"
#include "GameplayExperiencesLog.h"
#include "ModularAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Developer/ExperienceGameSettings.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperiencePawnExtensionComponent)

class FLifetimeProperty;
class UActorComponent;

const FName UExperiencePawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");

UExperiencePawnExtensionComponent::UExperiencePawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	PawnData = nullptr;
	AbilitySystem = nullptr;
}

void UExperiencePawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for changes to all features
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Notifies state manager that we've spawned, then try rest of default initialization
	ensure(TryToChangeInitState(UExperienceManagerSubsystem::Get()->GetTag_Spawned()));
	CheckDefaultInitialization();
}

void UExperiencePawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	
	Super::EndPlay(EndPlayReason);
}

void UExperiencePawnExtensionComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf(Pawn != nullptr, TEXT("PawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> PawnExtensionComponents;
	Pawn->GetComponents(StaticClass(), PawnExtensionComponents);
	ensureAlwaysMsgf(PawnExtensionComponents.Num() == 1, TEXT("Only one PawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// Register with the init state system early, this will only work if this is a game world
	RegisterInitStateFeature();
}

void UExperiencePawnExtensionComponent::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on
	CheckDefaultInitializationForImplementers();

	// Try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(UExperienceManagerSubsystem::Get()->StateChain);
}

bool UExperiencePawnExtensionComponent::CanChangeInitState(
	UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);
	APawn* Pawn = GetPawn<APawn>();
	const UExperienceManagerSubsystem* ExpMgr = UExperienceManagerSubsystem::Get();

	// None -> Spawned
	if (!CurrentState.IsValid() && DesiredState == ExpMgr->GetTag_Spawned())
	{
		// As long as we're on a valid pawn, we count as spawned
		if (IsValid(Pawn))
		{
			EXPERIENCE_LOG(Log, TEXT("Transitioning from %s to %s"), *CurrentState.ToString(), *DesiredState.ToString());
			return true;
		}
	}

	// Spawned -> Available
	if (CurrentState == ExpMgr->GetTag_Spawned() && DesiredState == ExpMgr->GetTag_Available())
	{
		// Pawn data is required
		if (PawnData == nullptr)
		{
			EXPERIENCE_LOG(Log, TEXT("PawnData is required to transition from %s to %s"), *CurrentState.ToString(), *DesiredState.ToString());
			return false;
		}

		const bool bHasAuthority = Pawn->HasAuthority();
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

		if (bHasAuthority || bIsLocallyControlled)
		{
			// Check for a valid controller
			if (!GetController<AController>())
			{
				EXPERIENCE_LOG(Log, TEXT("No controller found to transition from %s to %s"), *CurrentState.ToString(), *DesiredState.ToString());
				return false;
			}
		}

		EXPERIENCE_LOG(Log, TEXT("Transitioning from %s to %s"), *CurrentState.ToString(), *DesiredState.ToString());
		return true;
	}

	// Available -> Initialized
	if (CurrentState == ExpMgr->GetTag_Available() && DesiredState == ExpMgr->GetTag_Initialized())
	{
		// Transition to initialize if all features have their data available
		const bool bAllFeaturesAvailable = Manager->HaveAllFeaturesReachedInitState(Pawn, ExpMgr->GetTag_Available());

		if (bAllFeaturesAvailable)
		{
			EXPERIENCE_LOG(Log, TEXT("Transitioning from %s to %s"), *CurrentState.ToString(), *DesiredState.ToString());
		}
		else
		{
			EXPERIENCE_LOG(Log, TEXT("Still waiting for all features to be available"));
		}

		return bAllFeaturesAvailable;
	}

	// Initialized -> Ready
	if (CurrentState == ExpMgr->GetTag_Initialized() && DesiredState == ExpMgr->GetTag_Ready())
	{
		EXPERIENCE_LOG(Log, TEXT("Transitioning from %s to %s"), *CurrentState.ToString(), *DesiredState.ToString());
		return true;
	}

	EXPERIENCE_LOG(Warning, TEXT("Invalid transition from %s to %s"), *CurrentState.ToString(), *DesiredState.ToString());
	return false;
}

void UExperiencePawnExtensionComponent::HandleChangeInitState(
	UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// Nothing to do here.
	// Will be handled by other components listening to the state

	EXPERIENCE_LOG(Log, TEXT("---- CurrentState=%s --> DesiredState=%s"), *CurrentState.ToString(), *DesiredState.ToString());
}

void UExperiencePawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If any feature is now in DataAvailable, see if we should transition to DataInitialized
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == UExperienceManagerSubsystem::Get()->GetTag_Available())
		{
			CheckDefaultInitialization();
		}
	}
}

void UExperiencePawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PawnData);
}

void UExperiencePawnExtensionComponent::SetPawnData(const UExperiencePawnData* InPawnData)
{
	check(InPawnData);
	APawn* Pawn = GetPawnChecked<APawn>();

	if (Pawn->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		EXPERIENCE_LOG(Error, TEXT("Trying to set PawnData '%s' on pawn '%s' that already has valid PawnData '%s'."),
			*GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));

		return;
	}

	PawnData = InPawnData;
	Pawn->ForceNetUpdate();

	CheckDefaultInitialization();
}

void UExperiencePawnExtensionComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}

void UExperiencePawnExtensionComponent::HandleControllerChanged()
{
	if (AbilitySystem && (AbilitySystem->GetAvatarActor() == GetPawnChecked<APawn>()))
	{
		ensure(AbilitySystem->AbilityActorInfo->OwnerActor == AbilitySystem->GetOwnerActor());
		if (AbilitySystem->GetOwnerActor() == nullptr)
		{
			
		}
		else
		{
			AbilitySystem->RefreshAbilityActorInfo();
		}
	}

	CheckDefaultInitialization();
}

void UExperiencePawnExtensionComponent::InitializeAbilitySystem(UAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	if (!ensureAlwaysMsgf(InASC, TEXT("Invalid ability system component")))
	{
		return;
	}
	
	check(InOwnerActor);

	// Check if the ability system component hasn't changed
	if (AbilitySystem == InASC)
	{
		return;
	}

	// Clean up the old ability system component
	if (AbilitySystem)
	{
		UninitializeAbilitySystem();
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	const AActor* ExistingAvatar = InASC->GetAvatarActor();

	EXPERIENCE_LOG(Verbose, TEXT("Setting up ASC '%s' on pawn (%s) owner (%s), existing (%s)"),
		*GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	if ((ExistingAvatar != nullptr) &&
		(ExistingAvatar != Pawn))
	{
		EXPERIENCE_LOG(Log, TEXT("Existing avatar (authority=%d"), ExistingAvatar->HasAuthority());

		// Kick out the existing avatar
		// Mostly happens on clients if they're lagging.
		ensure(!ExistingAvatar->HasAuthority());

		if (UExperiencePawnExtensionComponent* OtherExtensionComp = FindPawnExtensionComponent(ExistingAvatar))
		{
			OtherExtensionComp->UninitializeAbilitySystem();
		}
	}

	AbilitySystem = InASC;
	AbilitySystem->InitAbilityActorInfo(InOwnerActor, Pawn);

	if (ensure(PawnData))
	{
		if (UModularAbilitySystemComponent* ModularAbilitySystem = Cast<UModularAbilitySystemComponent>(AbilitySystem))
		{
			ModularAbilitySystem->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
		}
	}

	
	UAbilitySystemGlobals::Get().GetAttributeSetInitter()->InitAttributeSetDefaults(AbilitySystem, "Health", 1, true);

	OnAbilitySystemInitialized.Broadcast();
}

void UExperiencePawnExtensionComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystem)
	{
		return;
	}

	// Uninitialize the ability system component if we're still the avatar actor
	// Otherwise another pawn already did it when they became the avatar actor
	if (AbilitySystem->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;

		AbilitySystem->CancelAbilities(nullptr, &AbilityTypesToIgnore);

		if (UModularAbilitySystemComponent* ModularAbilitySystem = Cast<UModularAbilitySystemComponent>(AbilitySystem))
		{
			ModularAbilitySystem->ClearAbilityInput();
		}

		AbilitySystem->RemoveAllGameplayCues();

		if (AbilitySystem->GetOwnerActor() != nullptr)
		{
			AbilitySystem->SetAvatarActor(nullptr);
		}
		else
		{
			AbilitySystem->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	AbilitySystem = nullptr;
}

void UExperiencePawnExtensionComponent::HandlePlayerStateReplicated()
{
	CheckDefaultInitialization();
}

void UExperiencePawnExtensionComponent::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

void UExperiencePawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(
	FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystem)
	{
		Delegate.Execute();
	}
}

void UExperiencePawnExtensionComponent::OnAbilitySystemUninitialized_Register(
	FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}
