// Copyright © 2024 Playton. All Rights Reserved.


#include "ModularExperienceGameMode.h"

#include "ExperienceDefinition.h"
#include "Components/ExperienceManagerComponent.h"
#include "GameFramework/ExperiencePlayerState.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "ModularExperienceGameMode"
#endif

#include "ExperiencePawnData.h"
#include "ExperienceWorldSettings.h"
#include "GameplayExperiencesLog.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularExperienceGameMode)

//////////////////////////////////////////////////////////////////////////
/// AModularExperienceGameMode

AModularExperienceGameModeBase::AModularExperienceGameModeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerStateClass = AExperiencePlayerState::StaticClass();
}

const UExperiencePawnData* AModularExperienceGameModeBase::GetPawnDataForController(const AController* InController) const
{
	// Check if the pawn data has already ben set on the player state
	if (InController != nullptr)
	{
		if (const AExperiencePlayerState* ExperiencePS = InController->GetPlayerState<AExperiencePlayerState>())
		{
			
		}
	}

	// No pawn data found?
	// Fall back to the default pawn data OR the one for the current experience
	check(GameState);
	UExperienceManagerComponent* ExperienceMgr = GameState->FindComponentByClass<UExperienceManagerComponent>();
	check(ExperienceMgr);
	if (ExperienceMgr->IsExperienceLoaded())
	{
		const UExperienceDefinition* ExperienceDefinition = ExperienceMgr->GetLoadedExperience_Checked();
		if (ExperienceDefinition->DefaultPawnData != nullptr)
		{
			return ExperienceDefinition->DefaultPawnData;
		}

		// If none found, fall back to the default pawn data
		if (DefaultPawnData.IsValid())
		{
			return DefaultPawnData.LoadSynchronous();
		}
	}

	// No experience loaded yet?
	// Return nullptr so there is no pawn data to use
	return nullptr;
}

void AModularExperienceGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Wait for next tick to handle match assignment if not expecting one
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne);
}

void AModularExperienceGameModeBase::HandleMatchAssignmentIfNotExpectingOne()
{
	FPrimaryAssetId ExperienceId;
	FString ExperienceIdSource;

	auto ShouldContinue = [ExperienceId]()->bool
	{
		return !ExperienceId.IsValid();
	};

	// Precedence order (highest wins)
	//  – Matchmaking assignment (if present)
	//  – URL Options override
	//  – Developer Settings (PIE only)
	//  – Command Line override
	//  – World Settings
	//  – Dedicated server
	//  – Default experience

	UWorld* World = GetWorld();

	// Matchmaking assignment

	// URL Options override
	if (ShouldContinue() && UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
	{
		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UExperienceDefinition::StaticClass()->GetFName()), FName(*ExperienceFromOptions));
		ExperienceIdSource = TEXT("URL Options");
	}

	// Developer Settings (PIE only)
	if (ShouldContinue() && World->IsPlayInEditor())
	{
		
	}

	// Command Line override
	if (ShouldContinue())
	{
		FString ExperienceFromCommandLine;
		if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
		{
			ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
			if (!ExperienceId.PrimaryAssetType.IsValid())
			{
				ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UExperienceDefinition::StaticClass()->GetFName()), FName(*ExperienceFromCommandLine));
			}
			ExperienceIdSource = TEXT("Command Line");
		}
	}

	// World Settings
	if (ShouldContinue())
	{
		if (AExperienceWorldSettings* ExperienceSettings = Cast<AExperienceWorldSettings>(GetWorldSettings()))
		{
			ExperienceId = ExperienceSettings->GetDefaultGameplayExperience();
			ExperienceIdSource = TEXT("World Settings");
		}
		else
		{
			ensureMsgf(false, TEXT("The world settings for this world are not of type AExperienceWorldSettings. This is required to load an experience."));
		}
	}

	// Validate the experience ID
	UAssetManager& AssetManager = UAssetManager::Get();
	FAssetData Dummy;
	if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, Dummy))
	{
		EXPERIENCE_LOG(Error, TEXT("Wanted to use experience '%s' but couldn't find it, falling back to the default"), *ExperienceId.ToString());
		ExperienceId = FPrimaryAssetId();
	}

	// Default experience
	if (ShouldContinue())
	{
		if (TryDedicatedServerLogin())
		{
			return;
		}

		ExperienceId = AssetManager.GetPrimaryAssetIdForPath(DefaultExperience.ToSoftObjectPath());
		if (!ExperienceId.IsValid())
		{
			EXPERIENCE_LOG(Error, TEXT("%s.DefaultExperience is %s but that failed to resolve into an asset ID."),
				*GetPathNameSafe(this), *DefaultExperience.ToString());
		}

		ExperienceIdSource = TEXT("Default Experience");
	}

	OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
}

void AModularExperienceGameModeBase::OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource)
{
	if (ExperienceId.IsValid())
	{
		EXPERIENCE_LOG(Log, TEXT("Identified experience '%s' from %s"), *ExperienceId.ToString(), *ExperienceIdSource);

		UExperienceManagerComponent* ExperienceMgr = GameState->FindComponentByClass<UExperienceManagerComponent>();
		check(ExperienceMgr);

		ExperienceMgr->SetCurrentExperience(ExperienceId);
	}
	else
	{
		EXPERIENCE_LOG(Error, TEXT("No experience assignment found, continuing with no experience loaded"));
	}
}

void AModularExperienceGameModeBase::OnExperienceLoaded(const UExperienceDefinition* CurrentExperience)
{
	// Spawn any actors that need to be spawned
	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = Cast<APlayerController>(*It);
		if (PC && PC->GetPawn() == nullptr)
		{
			if (PlayerCanRestart(PC))
			{
				RestartPlayer(PC);
			}
		}
	}
}

bool AModularExperienceGameModeBase::IsExperienceLoaded() const
{
	check(GameState);
	const UExperienceManagerComponent* ExperienceMgr = GameState->FindComponentByClass<UExperienceManagerComponent>();
	check(ExperienceMgr);

	return ExperienceMgr->IsExperienceLoaded();
}

bool AModularExperienceGameModeBase::TryDedicatedServerLogin()
{
	return false;
}

void AModularExperienceGameModeBase::InitGameState()
{
	Super::InitGameState();

	UExperienceManagerComponent* ExperienceMgr = GameState->FindComponentByClass<UExperienceManagerComponent>();
	check(ExperienceMgr);

	ExperienceMgr->CallOrRegister_OnExperienceLoaded(FOnExperienceLoaed::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

UClass* AModularExperienceGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UExperiencePawnData* PawnData = GetPawnDataForController(InController))
	{
		if (PawnData->PawnClass)
		{
			return PawnData->PawnClass;
		}
	}
	
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* AModularExperienceGameModeBase::SpawnDefaultPawnAtTransform_Implementation(
	AController* NewPlayer, const FTransform& SpawnTransform)
{
	return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
}

AActor* AModularExperienceGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	return Super::ChoosePlayerStart_Implementation(Player);
}

void AModularExperienceGameModeBase::RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset)
{
	if (bForceReset && Controller)
	{
		Controller->Reset();
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(PC, &APlayerController::ServerRestartPlayer_Implementation);
	}
}

bool AModularExperienceGameModeBase::ShouldSpawnAtStartSpot(AController* Player)
{
	return Super::ShouldSpawnAtStartSpot(Player);
}

void AModularExperienceGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Delay starting new players until the experience has been loaded
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

void AModularExperienceGameModeBase::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

bool AModularExperienceGameModeBase::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return ControllerCanRestart(Player);
}

bool AModularExperienceGameModeBase::UpdatePlayerStartSpot(
	AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	return true;
}

void AModularExperienceGameModeBase::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);

	OnGameModePlayerInitialized.Broadcast(this, C);
}

void AModularExperienceGameModeBase::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APlayerController* NewPC = Cast<APlayerController>(NewPlayer))
		{
			if (PlayerCanRestart(NewPC))
			{
				RequestPlayerRestartNextFrame(NewPlayer, false);
			}
			else
			{
				EXPERIENCE_LOG(Verbose, TEXT("Failed to restart player %s and PlayerCanRestart returned false."), *GetPathNameSafe(NewPlayer));
			}
		}
		else
		{
			RequestPlayerRestartNextFrame(NewPlayer, false);
		}
	}
	else
	{
		EXPERIENCE_LOG(Verbose, TEXT("Failed to restart player %s and GetDefaultPawnClassForController returned nullptr."), *GetPathNameSafe(NewPlayer));
	}
}

bool AModularExperienceGameModeBase::ControllerCanRestart(AController* Controller)
{
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (!Super::PlayerCanRestart_Implementation(PC))
		{
			return false;
		}
	}
	else
	{
		if ((Controller == nullptr) || Controller->IsPendingKillPending())
		{
			return false;
		}
	}

	return true;
}



#if WITH_EDITOR
EDataValidationResult AModularExperienceGameModeBase::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (PlayerStateClass == nullptr)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("NoPlayerStateClass", "PlayerStateClass must be set."));
	}
	else if (!PlayerStateClass->IsChildOf(AExperiencePlayerState::StaticClass()))
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("InvalidPlayerStateClass", "PlayerStateClass must be a subclass of AExperiencePlayerState."));
	}

	return Result;
}
#endif

//////////////////////////////////////////////////////////////////////////
/// AModularExperienceGameModeBase

AModularExperienceGameMode::AModularExperienceGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#undef LOCTEXT_NAMESPACE