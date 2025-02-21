// Copyright © 2024 Playton. All Rights Reserved.


#include "AssetDefinition_InputConfig.h"

#include "Input/InputConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_InputConfig)

#define LOCTEXT_NAMESPACE "GameplayExperiencesEditorPlugin"

FText UAssetDefinition_InputConfig::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDisplayName_InputConfig", "Input Config");
}

FLinearColor UAssetDefinition_InputConfig::GetAssetColor() const
{
	return FLinearColor(FColor(255, 255, 127));
}

TSoftClassPtr<> UAssetDefinition_InputConfig::GetAssetClass() const
{
	return UInputConfig::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_InputConfig::GetAssetCategories() const
{
	static const auto Categories = { FAssetCategoryPath(EAssetCategoryPaths::Input) };
	return Categories;
}

#undef LOCTEXT_NAMESPACE