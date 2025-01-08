// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "InputConfig.generated.h"

class UInputAction;

/**
 * Struct used to map an UInputAction to a Gameplay Tag.
 */
USTRUCT(BlueprintType)
struct FInputConfig_ActionBinding
{
	GENERATED_BODY()

public:
	/** The Input action to map. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<const UInputAction> InputAction = nullptr;

	/** The Gameplay Tag to map the Input action to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (Categories = "Input.InputTag"))
	FGameplayTag GameplayTag;
};

/**
 * Non-mutable data asset containing input configuration data.
 * Used to map out certain UInputActions to Gameplay Tags.
 */
UCLASS(BlueprintType, Const)
class GAMEPLAYEXPERIENCESRUNTIME_API UInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UInputConfig(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Retrieves a native input action by its Gameplay Tag. */
	const UInputAction* FindNativeInputActionByTag(const FGameplayTag& Tag, bool bLogNotFound = true) const;

	/** Retrieves an ability input action by its Gameplay Tag. */
	const UInputAction* FindAbilityInputActionByTag(const FGameplayTag& Tag, bool bLogNotFound = true) const;

public:
	/** List of native input actions used by the owner. These input actions are mapped to Gameplay Tags but must be manually bound. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (TitleProperty = "{InputAction} -> {GameplayTag}"))
	TArray<FInputConfig_ActionBinding> NativeInputActions;

	/** List of ability input actions used by the owner. These input actions are mapped to Gameplay Tags and are automatically bound to abilities matching the Gameplay Tag. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (TitleProperty = "{InputAction} -> {GameplayTag}"))
	TArray<FInputConfig_ActionBinding> AbilityInputActions;

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	//~ End UObject Interface
#endif

private:
	const UInputAction* FindInputAction(const FGameplayTag& Tag, const TArray<FInputConfig_ActionBinding>& InputActions, bool bLogNotFound) const;
};
