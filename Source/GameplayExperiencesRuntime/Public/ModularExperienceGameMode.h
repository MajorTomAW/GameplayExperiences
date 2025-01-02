// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "ModularGameMode.h"

#include "ModularExperienceGameMode.generated.h"

class UExperienceDefinition;
class AActor;
class AController;
class AGameModeBase;
class APawn;
class APlayerController;
class UClass;
class UExperiencePawnData;
class UObject;
struct FFrame;
struct FPrimaryAssetId;

/**
 * Post-login event, triggered when a player or bot joins the game, as well as after seamless and non-seamless travel.
 * Called after the player has finished initialization.
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

/**
 * Game mode for a modular experience. 
 */
UCLASS(Config = Game)
class GAMEPLAYEXPERIENCESRUNTIME_API AModularExperienceGameModeBase : public AModularGameModeBase
{
	GENERATED_BODY()
	
public:
	AModularExperienceGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Retrieves the pawn data for the given controller. */
	UFUNCTION(BlueprintCallable, Category = "Experience")
	const UExperiencePawnData* GetPawnDataForController(const AController* InController) const;

	/**
	 * Requests a restart (respawn) of the specified controller the next frame.
	 * @param bForceReset If true, the controller will be reset and abandon the current pawn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Experience")
	void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

public:
	//~ Begin AGameModeBase Interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	virtual void GenericPlayerInitialization(AController* C) override;
	virtual void FailedToRestartPlayer(AController* NewPlayer) override;
	//~ End AGameModeBase Interface

	/** Agnostic version of PlayerCanRestart that can be used for both player and AI controllers. */
	virtual bool ControllerCanRestart(AController* Controller);

	/** Delegate called when a player or bot joins the game. */
	FOnPlayerInitialized OnGameModePlayerInitialized;

protected:
	void OnExperienceLoaded(const UExperienceDefinition* CurrentExperience);
	bool IsExperienceLoaded() const;

	void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);

	void HandleMatchAssignmentIfNotExpectingOne();

	virtual bool TryDedicatedServerLogin();

	/** Override to return the experience to load from the developer settings. (will be project-specific) */
	virtual FPrimaryAssetId GetExperienceFromDeveloperSettings() const { return FPrimaryAssetId(); }

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	//~ End UObject Interface
#endif

protected:
	/** Default pawn data to fall back to if no pawn data is found. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = Classes, AdvancedDisplay)
	TSoftObjectPtr<UExperiencePawnData> DefaultPawnData;

	/** Default experience to load if no experience is found. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = Classes, AdvancedDisplay)
	TSoftObjectPtr<UExperienceDefinition> DefaultExperience;
};

/**
 * Game mode for a modular experience. 
 */
UCLASS(Config = Game)
class AModularExperienceGameMode : public AModularGameMode
{
	GENERATED_BODY()

public:
	AModularExperienceGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
