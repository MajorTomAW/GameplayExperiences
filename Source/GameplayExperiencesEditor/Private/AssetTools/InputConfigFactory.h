// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"

#include "InputConfigFactory.generated.h"

class UInputConfig;

UCLASS(HideCategories = Object)
class UInputConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UInputConfigFactory();

	//~ Begin UFactory Interface
	virtual FText GetDisplayName() const override;
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	//~ End UFactory Interface

protected:
	UPROPERTY()TSubclassOf<UInputConfig> SelectedClass;
};
