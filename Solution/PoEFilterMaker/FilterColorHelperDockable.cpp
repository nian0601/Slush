#include "stdafx.h"

#include "FilterColorHelperDockable.h"
#include <FW_FileSystem.h>

FilterColorHelperDockable::FilterColorHelperDockable()
	: Dockable(false)
{
}

void FilterColorHelperDockable::OnBuildUI()
{
	ImGui::ColorPicker4("Picker", myColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview);

	FW_String text;
	text += static_cast<int>(myColor[0] * 255.f);
	text += " ";
	text += static_cast<int>(myColor[1] * 255.f);
	text += " ";
	text += static_cast<int>(myColor[2] * 255.f);
	text += " ";
	text += static_cast<int>(myColor[3] * 255.f);

	if (ImGui::InputText("Text", &text, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
	{
		FW_GrowingArray<FW_String> words;
		FW_FileSystem::SplitLineOnSpace(text, words);

		if (words.Count() == 3 || words.Count() == 4)
		{
			myColor[0] = FW_FileSystem::GetInt(words[0]) / 255.f;
			myColor[1] = FW_FileSystem::GetInt(words[1]) / 255.f;
			myColor[2] = FW_FileSystem::GetInt(words[2]) / 255.f;
			myColor[3] = 1.f;
			if (words.Count() == 4)
				myColor[3] = FW_FileSystem::GetInt(words[3]) / 255.f;
		}
		else
		{
			SLUSH_ERROR("Invalid Color-format");
		}
	}
}