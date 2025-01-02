// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "Styling/SlateStyle.h"

class FGameplayExperienceEditorStyle : public FSlateStyleSet
{
public:
	FGameplayExperienceEditorStyle();
	virtual ~FGameplayExperienceEditorStyle();
	friend class FGameplayExperiencesEditorModule;

	/** Returns the singleton instance of the style set */
	static FGameplayExperienceEditorStyle& Get();

protected:
	void Initialize();
	static void Shutdown();

private:
	static TUniquePtr<FGameplayExperienceEditorStyle> StyleSet;
};