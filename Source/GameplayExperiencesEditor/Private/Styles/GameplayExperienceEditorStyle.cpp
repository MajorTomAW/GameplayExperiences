// Copyright © 2024 Playton. All Rights Reserved.


#include "GameplayExperienceEditorStyle.h"

#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

TUniquePtr<FGameplayExperienceEditorStyle> FGameplayExperienceEditorStyle::StyleSet = nullptr;

FGameplayExperienceEditorStyle::FGameplayExperienceEditorStyle()
	: FSlateStyleSet("GameplayExperienceEditorStyle")
{
}

FGameplayExperienceEditorStyle::~FGameplayExperienceEditorStyle()
{
	Shutdown();
}

FGameplayExperienceEditorStyle& FGameplayExperienceEditorStyle::Get()
{
	if (!StyleSet.IsValid())
	{
		StyleSet = MakeUnique<FGameplayExperienceEditorStyle>();
		StyleSet->Initialize();
	}

	return *StyleSet.Get();
}

void FGameplayExperienceEditorStyle::Initialize()
{
	FSlateStyleSet::SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	FSlateStyleSet::SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	Set("ClassIcon.ExperiencePawnData", new IMAGE_BRUSH_SVG("Starship/AssetIcons/Pawn_16", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.ExperiencePawnData", new IMAGE_BRUSH_SVG("Starship/AssetIcons/Pawn_64", CoreStyleConstants::Icon64x64));

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

void FGameplayExperienceEditorStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet);
		StyleSet.Reset();
	}
}
