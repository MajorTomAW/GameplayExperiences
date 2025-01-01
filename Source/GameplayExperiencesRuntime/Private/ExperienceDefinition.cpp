// Copyright Epic Games, Inc. All Rights Reserved.


#include "ExperienceDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "GameFeatureAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceDefinition)

#define LOCTEXT_NAMESPACE "GameplayExperiences"

UExperienceDefinition::UExperienceDefinition()
{
}

#if WITH_EDITORONLY_DATA
void UExperienceDefinition::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action : FeatureActions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif
#if WITH_EDITOR
EDataValidationResult UExperienceDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	{ // Validate the Feature Actions
		int32 EntryIndex = 0;
		for (const UGameFeatureAction* Action : FeatureActions)
		{
			if (Action)
			{
				const EDataValidationResult ChildResult = Action->IsDataValid(Context);
				Result = CombineDataValidationResults(Result, ChildResult);
			}
			else
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("ActionEntryIsNull", "Null entry at index {0} in Actions"), FText::AsNumber(EntryIndex)));
			}

			++EntryIndex;
		}
	}

	{ // Validate the Plugin Dependencies
		int32 EntryIndex = 0;
		for (const auto& Plugin : GameFeaturesToEnable)
		{
			if (!Plugin.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("PluginEntryIsNull", "Null entry at index {0} in GameFeaturesToEnable"), FText::AsNumber(EntryIndex)));
			}

			++EntryIndex;
		}
	}

	// Make sure users didn't subclass from a BP of this
	// It is expected to subclass once in BP, but not twice or more
	if (!GetClass()->IsNative())
	{
		const UClass* ParentClass = GetClass()->GetSuperClass();

		// Find the native parent class
		const UClass* FirstNativeParent = ParentClass;
		while ((FirstNativeParent != nullptr) && !FirstNativeParent->IsNative())
		{
			FirstNativeParent = FirstNativeParent->GetSuperClass();
		}

		if (FirstNativeParent != ParentClass)
		{
			Context.AddError(FText::Format(
				LOCTEXT("ExperienceInheritanceNotSupported", "Blueprint subclasses of Blueprint experiences is not currently supported."
				"(Use composition via FeatureActionSets instead.) Parent class was {0} but should be {1}."),
				FText::AsCultureInvariant(GetPathNameSafe(ParentClass)),
				FText::AsCultureInvariant(GetPathNameSafe(FirstNativeParent))));

			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif

TArray<FName> UExperienceDefinition::GetGameFeaturePluginDependencies() const
{
	TArray<FName> URLs;
	for (const auto& Plugin : GameFeaturesToEnable)
	{
		URLs.Add(*Plugin.GetPluginName());
	}
	return URLs;
}

#undef LOCTEXT_NAMESPACE
