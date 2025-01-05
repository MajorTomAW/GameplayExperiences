// Copyright © 2024 Playton. All Rights Reserved.

#pragma once
#include "AssetRegistry/AssetRegistryModule.h"

namespace UE::GameplayExperiences::Editor
{
	inline bool DoesClassHaveSubtypes(UClass* Class)
	{
		// Search native classes
		for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
		{
			if (ClassIt->IsNative() && ClassIt->IsChildOf(Class))
			{
				if (ClassIt->HasAnyClassFlags(CLASS_Abstract | CLASS_HideDropDown | CLASS_Deprecated | CLASS_NewerVersionExists))
				{
					continue;
				}

				UObject* CDO = ClassIt->GetDefaultObject();
				if (CDO && CDO->GetClass() != Class)
				{
					return true;
				}
			}
		}

		// Search BPs via asset registry
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		FARFilter Filter;
		Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
		Filter.bRecursiveClasses = true;
		TArray<FAssetData> BlueprintAssetData;
		AssetRegistry.GetAssets(Filter, BlueprintAssetData);

		for (FAssetData& Asset : BlueprintAssetData)
		{
			FAssetDataTagMapSharedView::FFindTagResult Result = Asset.TagsAndValues.FindTag(TEXT("NativeParentClass"));
			if (Result.IsSet())
			{
				const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(Result.GetValue());
				if (UClass* ParentClass = FindObjectSafe<UClass>(nullptr, *ClassObjectPath, true))
				{
					if (ParentClass->IsChildOf(Class))
					{
						return true;
					}
				}
			}
		}

		return false;
	}
}
