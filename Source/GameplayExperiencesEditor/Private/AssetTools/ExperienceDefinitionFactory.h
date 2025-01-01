// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/BlueprintFactory.h"

#include "ExperienceDefinitionFactory.generated.h"

UCLASS(HideCategories = Object)
class UExperienceDefinitionFactory : public UBlueprintFactory
{
	GENERATED_BODY()

public:
	UExperienceDefinitionFactory();

protected:
	//~ Begin UFactory Interface
	virtual FText GetDisplayName() const override;
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	//~ End UFactory Interface
};
