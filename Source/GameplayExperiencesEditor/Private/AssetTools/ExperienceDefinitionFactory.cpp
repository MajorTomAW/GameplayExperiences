// Copyright Epic Games, Inc. All Rights Reserved.


#include "ExperienceDefinitionFactory.h"

#include "ClassViewerModule.h"
#include "ExperienceDefinition.h"
#include "GameplayExperiencesClassFilter.h"
#include "KismetCompilerModule.h"
#include "Kismet2/KismetEditorUtilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceDefinitionFactory)

#define LOCTEXT_NAMESPACE "GameFeaturesEditor"

UExperienceDefinitionFactory::UExperienceDefinitionFactory()
{
	SupportedClass = UExperienceDefinition::StaticClass();
	ParentClass = UExperienceDefinition::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;

	BlueprintType = BPTYPE_Normal;

	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(ParentClass, DerivedClasses, false);
	bSkipClassPicker = DerivedClasses.Num() == 0;

	OnConfigurePropertiesDelegate.BindLambda([](FClassViewerInitializationOptions* Options)
	{
		Options->bShowNoneOption = false;
		Options->ClassFilters.Add(MakeShareable(new FGameplayExperiencesClassFilter(UExperienceDefinition::StaticClass())));
		Options->ExtraPickerCommonClasses.Reset();
	});
}

FText UExperienceDefinitionFactory::GetDisplayName() const
{
	return LOCTEXT("ExperienceDefinitionFactoryDisplayName", "Experience Definition");
}

bool UExperienceDefinitionFactory::ConfigureProperties()
{
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(ParentClass, DerivedClasses, false);
	if (bSkipClassPicker || DerivedClasses.Num() == 0)
	{
		// If there is only one derived class, skip the class picker
		return true;
	}
	
	return Super::ConfigureProperties();
}

UObject* UExperienceDefinitionFactory::FactoryCreateNew(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	if ((ParentClass == nullptr) || !FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass))
	{
		FFormatNamedArguments Args;
		Args.Add( TEXT("ClassName"), (ParentClass != nullptr) ? FText::FromString( ParentClass->GetName() ) : LOCTEXT("Null", "(null)") );
		FMessageDialog::Open( EAppMsgType::Ok, FText::Format( LOCTEXT("CannotCreateBlueprintFromClass", "Cannot create a blueprint based on the class '{ClassName}'."), Args ) );
		return nullptr;
	}

	UClass* BlueprintClass = nullptr;
	UClass* BlueprintGeneratedClass = nullptr;

	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
	KismetCompilerModule.GetBlueprintTypesForClass(ParentClass, BlueprintClass, BlueprintGeneratedClass);

	UBlueprint* Blueprint =
		FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, InName, BlueprintType, BlueprintClass, BlueprintGeneratedClass, CallingContext);

	// Mark the Blueprint as Data-Only
	Blueprint->bRecompileOnLoad = false;

	return Blueprint;
}

#undef LOCTEXT_NAMESPACE
