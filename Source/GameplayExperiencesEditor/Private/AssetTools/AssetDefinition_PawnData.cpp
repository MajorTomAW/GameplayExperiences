// Copyright © 2024 Playton. All Rights Reserved.


#include "AssetDefinition_PawnData.h"

#include "ExperiencePawnData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_PawnData)

#define LOCTEXT_NAMESPACE "GameplayExperiencesEditor"

FText UAssetDefinition_PawnData::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDisplayName", "Pawn Data");
}

FLinearColor UAssetDefinition_PawnData::GetAssetColor() const
{
	return FLinearColor(FColor(201, 29, 85));
}

TSoftClassPtr<> UAssetDefinition_PawnData::GetAssetClass() const
{
	return UExperiencePawnData::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_PawnData::GetAssetCategories() const
{
	static const auto Categories =
	{
		FAssetCategoryPath(LOCTEXT("CategoryLabel", "Game Features"))
	};

	return Categories;
}

#undef LOCTEXT_NAMESPACE