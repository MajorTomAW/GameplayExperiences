// Copyright © 2024 Playton. All Rights Reserved.


#include "Components/ExperienceHeroComponent.h"

#include "AbilitySystemGlobals.h"
#include "EnhancedInputSubsystems.h"
#include "ExperienceGameFrameworkCallouts.h"
#include "ExperienceManagerSubsystem.h"
#include "GameplayExperiencesLog.h"
#include "InputMappingContext.h"
#include "Components/ExperiencePawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/ExperiencePlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Misc/UObjectToken.h"
#include "Input/InputConfig.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceHeroComponent)

const FName UExperienceHeroComponent::NAME_ActorFeatureName("Hero");

UExperienceHeroComponent::UExperienceHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bReadyToBindInputs(false)
{
}

bool UExperienceHeroComponent::CanChangeInitState(
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
			return true;
		}
	}

	// Spawned -> Available
	else if (CurrentState == ExpMgr->GetTag_Spawned() && DesiredState == ExpMgr->GetTag_Available())
	{
		// The player state is required
		if (!GetPlayerState<APlayerState>())
		{
			return false;
		}

		// Check for simulated proxies
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			const AController* C = GetController<AController>();

			const bool bHasControllerParedWithPS = (C != nullptr) && \
				(C->PlayerState != nullptr) && \
				(C->PlayerState->GetOwner() == C);

			if (!bHasControllerParedWithPS)
			{
				return false;
			}
		}

		// Check for networking
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBotControlled = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBotControlled)
		{
			const APlayerController* PC = GetController<APlayerController>();

			if (!Pawn->InputComponent || !PC || !PC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}

	// Available -> Initialized
	else if (CurrentState == ExpMgr->GetTag_Available() && DesiredState == ExpMgr->GetTag_Initialized())
	{
		// Wait for player state and extension component
		APlayerState* PS = GetPlayerState<APlayerState>();
		return PS && Manager->HasFeatureReachedInitState(Pawn, UExperiencePawnExtensionComponent::NAME_ActorFeatureName, ExpMgr->GetTag_Initialized());
	}

	// Initialized -> Ready
	else if (CurrentState == ExpMgr->GetTag_Initialized() && DesiredState == ExpMgr->GetTag_Ready())
	{
		return true;
	}

	return false;
}

void UExperienceHeroComponent::HandleChangeInitState(
	UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	const UExperienceManagerSubsystem* ExpMgr = UExperienceManagerSubsystem::Get();

	if (CurrentState == ExpMgr->GetTag_Available() && DesiredState == ExpMgr->GetTag_Initialized())
	{
		const APawn* Pawn = GetPawn<APawn>();
		AExperiencePlayerState* PS = GetPlayerState<AExperiencePlayerState>();
		if (!ensure(Pawn && PS))
		{
			return;
		}

		const UExperiencePawnData* PawnData = nullptr;

		if (UExperiencePawnExtensionComponent* PawnExtComp = UExperiencePawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtComp->GetPawnData();
			PawnExtComp->InitializeAbilitySystem(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PS), PS);
		}

		if (Pawn->InputComponent != nullptr)
		{
			InitializePlayerInput(Pawn->InputComponent);
		}
	}
}

void UExperienceHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UExperiencePawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == UExperienceManagerSubsystem::Get()->GetTag_Initialized())
		{
			// If extension component is now initialized, see if we should transition to Initialized
			CheckDefaultInitialization();
		}
	}
}

void UExperienceHeroComponent::CheckDefaultInitialization()
{
	ContinueInitStateChain(UExperienceManagerSubsystem::Get()->StateChain);
}

void UExperienceHeroComponent::InitializePlayerInput(UInputComponent* InputComponent)
{
	check(InputComponent);

	EXPERIENCE_LOG(Log, TEXT("InitializePlayerInput on [%s]"), *GetNameSafe(GetOwner()));

	const APawn* Pawn = GetPawn<APawn>();
	if (Pawn == nullptr)
	{
		EXPERIENCE_LOG(Error, TEXT("InitializePlayerInput on [%s] called with no pawn!"), *GetNameSafe(GetOwner()));
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* InputSub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(InputSub);

	InputSub->ClearAllMappings();

	const UExperiencePawnExtensionComponent* PawnExtComp = UExperiencePawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	const UExperiencePawnData* PawnData = PawnExtComp ? PawnExtComp->GetPawnData() : nullptr;
	if (PawnData)
	{
		if (const UInputConfig* InputConfig = PawnData->InputConfig)
		{
			for (const auto& Mapping : DefaultInputMappings)
			{
				if (UInputMappingContext* IMC = Mapping.InputMapping.LoadSynchronous()) //@TODO: ??
				{
					if (!Mapping.bRegisterWithSettings)
					{
						continue;
					}

					if (UEnhancedInputUserSettings* Settings = InputSub->GetUserSettings())
					{
						Settings->RegisterInputMappingContext(IMC);
					}

					FModifyContextOptions Options = {};
					Options.bIgnoreAllPressedKeysUntilRelease = false;
					InputSub->AddMappingContext(IMC, Mapping.Priority, Options);
				}
			}

			OnInitializePlayerInput(InputComponent, InputConfig);
		}
		else
		{
			EXPERIENCE_LOG(Error, TEXT("Input Config is not set on [%s]"), *GetNameSafe(Pawn));
		}
	}
	else
	{
		// If we don't have pawn data, we need to wait until we do
		// This is because the input config is set on the pawn extension component
		// and we need to wait for that to be initialized before we can bind inputs
		EXPERIENCE_LOG(Warning, TEXT("PawnData is not set on [%s]"), *GetNameSafe(Pawn));
	}

	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

void UExperienceHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(UExperiencePawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(UExperienceManagerSubsystem::Get()->GetTag_Spawned()));
	CheckDefaultInitialization();
}

void UExperienceHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	
	Super::EndPlay(EndPlayReason);
}

void UExperienceHeroComponent::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn<APawn>())
	{
		EXPERIENCE_LOG(Error, TEXT("HeroComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("ExperienceHeroComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
			static const FName HeroMessageLogName = TEXT("ExperienceHeroComponent");

			FMessageLog(HeroMessageLogName).Error()
			->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
			->AddToken(FTextToken::Create(Message));

			FMessageLog(HeroMessageLogName).Open();
		}
#endif
	}
	else
	{
		RegisterInitStateFeature();
	}
}
