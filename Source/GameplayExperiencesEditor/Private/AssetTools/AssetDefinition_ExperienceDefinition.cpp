// Copyright Epic Games, Inc. All Rights Reserved.


#include "AssetDefinition_ExperienceDefinition.h"

#include "ExperienceDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_ExperienceDefinition)

#define LOCTEXT_NAMESPACE "GameplayExperiencesEditorPlugin"

FText UAssetDefinition_ExperienceDefinition::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDisplayName_ExperienceDefinition", "Experience Definition");
}

FLinearColor UAssetDefinition_ExperienceDefinition::GetAssetColor() const
{
	return FLinearColor(FColor( 63, 126, 255 ));
}

TSoftClassPtr<> UAssetDefinition_ExperienceDefinition::GetAssetClass() const
{
	return UExperienceDefinition::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_ExperienceDefinition::GetAssetCategories() const
{
	static const auto Categories =
	{
		FAssetCategoryPath(LOCTEXT("CategoryLabel_ExperienceDefinition", "Game Features"))
	};

	return Categories;
}

#undef LOCTEXT_NAMESPACE