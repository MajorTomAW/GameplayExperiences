// Copyright © 2024 Playton. All Rights Reserved.


#include "PawnDataFactory.h"

#include "ClassViewerModule.h"
#include "ExperiencePawnData.h"
#include "GameplayExperiencesClassFilter.h"
#include "Kismet2/SClassPickerDialog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PawnDataFactory)

#define LOCTEXT_NAMESPACE "GameplayExperiencesEditor"

UPawnDataFactory::UPawnDataFactory()
{
	SupportedClass = UExperiencePawnData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

FText UPawnDataFactory::GetDisplayName() const
{
	return LOCTEXT("PawnDataFactoryDisplayName", "Pawn Data");
}

bool UPawnDataFactory::ConfigureProperties()
{
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(SupportedClass, DerivedClasses, false);
	if (DerivedClasses.Num() == 0)
	{
		// If there is only one derived class, skip the class picker
		return true;
	}

	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;
	Options.bShowObjectRootClass = true;
	Options.bIsBlueprintBaseOnly = false;
	Options.bShowNoneOption = false;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;
	Options.ClassFilters.Add(MakeShareable(new FGameplayExperiencesClassFilter(UExperiencePawnData::StaticClass())));
	Options.ExtraPickerCommonClasses = CommonPawnDataClasses;

	const FText TitleText = LOCTEXT("CreatePawnDataTitle", "Pick Pawn Data Class");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UExperiencePawnData::StaticClass());

	if (bPressedOk)
	{
		SupportedClass = ChosenClass;
	}

	return bPressedOk;
}

UObject* UPawnDataFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(InClass->IsChildOf(UExperiencePawnData::StaticClass()));
	return NewObject<UExperiencePawnData>(InParent, InClass, InName, Flags);
}

#undef LOCTEXT_NAMESPACE
