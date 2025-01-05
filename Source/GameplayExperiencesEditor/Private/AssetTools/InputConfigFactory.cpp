// Copyright © 2024 Playton. All Rights Reserved.


#include "InputConfigFactory.h"

#include "ClassViewerModule.h"
#include "GameplayExperiencesClassFilter.h"
#include "GameplayExperiencesEditor/Classes/ExperienceEditorHelpers.h"
#include "Input/InputConfig.h"
#include "Kismet2/SClassPickerDialog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InputConfigFactory)

#define LOCTEXT_NAMESPACE "InputConfigFctory"

UInputConfigFactory::UInputConfigFactory()
{
	SupportedClass = UInputConfig::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

FText UInputConfigFactory::GetDisplayName() const
{
	return LOCTEXT("InputConfigFactoryDisplayName", "Input Config");
}

bool UInputConfigFactory::ConfigureProperties()
{
	if (!UE::GameplayExperiences::Editor::DoesClassHaveSubtypes(UInputConfig::StaticClass()))
	{
		return true;
	}

	SelectedClass = nullptr;

	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	TSharedPtr<FGameplayExperiencesClassFilter> Filter = MakeShareable(
		new FGameplayExperiencesClassFilter(UInputConfig::StaticClass()));
	Options.ClassFilters.Add(Filter.ToSharedRef());

	const FText TitleText = LOCTEXT("CreateInputConfigOptions", "Pick Class For Input Config Instance");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UInputConfig::StaticClass());

	if (bPressedOk)
	{
		SelectedClass = ChosenClass;
	}

	return bPressedOk;
}

UObject* UInputConfigFactory::FactoryCreateNew(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	if (SelectedClass != nullptr)
	{
		return NewObject<UInputConfig>(InParent, SelectedClass, InName, Flags | RF_Transactional, Context);
	}

	return NewObject<UInputConfig>(InParent, InClass, InName, Flags | RF_Transactional, Context);
}

#undef LOCTEXT_NAMESPACE
