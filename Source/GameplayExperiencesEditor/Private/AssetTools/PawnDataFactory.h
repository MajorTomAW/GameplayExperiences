// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"

#include "PawnDataFactory.generated.h"

UCLASS(HideCategories = Object, Config = Editor)
class UPawnDataFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPawnDataFactory();

protected:
	//~ Begin UFactory Interface
	virtual FText GetDisplayName() const override;
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	//~ End UFactory Interface

private:
	UPROPERTY(Config)
	TArray<TObjectPtr<UClass>> CommonPawnDataClasses;
};
