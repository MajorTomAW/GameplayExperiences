// Copyright © 2024 Playton. All Rights Reserved.


#include "Input/InputConfig.h"

#include "GameplayExperiencesLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InputConfig)

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "InputConfig"
#endif

UInputConfig::UInputConfig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UInputAction* UInputConfig::FindNativeInputActionByTag(const FGameplayTag& Tag, bool bLogNotFound) const
{
	return FindInputAction(Tag, NativeInputActions, bLogNotFound);
}

const UInputAction* UInputConfig::FindAbilityInputActionByTag(const FGameplayTag& Tag, bool bLogNotFound) const
{
	return FindInputAction(Tag, AbilityInputActions, bLogNotFound);
}

const UInputAction* UInputConfig::FindInputAction(const FGameplayTag& Tag, const TArray<FInputConfig_ActionBinding>& InputActions, bool bLogNotFound) const
{
	for (const auto& Action : InputActions)
	{
		if (Action.InputAction == nullptr)
		{
			continue;
		}

		if (!Action.GameplayTag.MatchesTagExact(Tag))
		{
			continue;
		}

		return Action.InputAction;
	}

	if (bLogNotFound)
	{
		EXPERIENCE_LOG(Warning, TEXT("Failed to find input action for tag '%s' on InputConfig '%s'"), *Tag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

#if WITH_EDITOR

EDataValidationResult UInputConfig::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	TArray<FInputConfig_ActionBinding> AllActions;
	AllActions.Append(NativeInputActions);
	AllActions.Append(AbilityInputActions);

	for (const auto& Action : AllActions)
	{
		if (Action.InputAction == nullptr)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("InputActionNull", "Input action is null in '{0}'"), FText::FromString(GetName())));
		}

		if (!Action.GameplayTag.IsValid())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("GameplayTagInvalid", "Gameplay tag is invalid in '{0}'"), FText::FromString(GetName())));
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
#endif
