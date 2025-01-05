#include "stdafx.h"

#include "FilterMakerDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"
#include <Core/Assets/AssetParser.h>
#include <FW_FileSystem.h>

//# Divine Orb Style
//Show
//Class "Currency"
//BaseType "Mirror" "Divine" "Perfect Jeweller's Orb"
//SetFontSize 35
//SetTextColor 255 0 0 255
//SetBorderColor 255 0 0 255
//SetBackgroundColor 255 255 255 255
//PlayAlertSound 6 300
//PlayEffect Red
//MinimapIcon 0 Red Star

void FilterElement::BuildUI()
{
	ImGui::PushID(this);

	if (myEnable)
	{
		if (ImGui::Button("X"))
			myEnable = false;

		ImGui::SameLine();

		OnBuildUI();
	}

	ImGui::PopID();
}

void FilterElement::Process(FW_FileProcessor& aProcessor)
{
	if (!myEnable)
		return;

	OnProcess(aProcessor);

	aProcessor.AddNewline();
}

void MinimapIcon::OnBuildUI()
{
	if (ImGui::CollapsingHeader("MinimapIcon"))
	{
		ImGui::Indent();
		ImGui::Combo("Size", &myIconSizeIndex, IconSizes, IM_ARRAYSIZE(IconSizes));
		ImGui::Combo("Shape", &myIconShapeIndex, IconShapes, IM_ARRAYSIZE(IconShapes));
		ImGui::Combo("Color", &myIconColorIndex, IconColors, IM_ARRAYSIZE(IconColors));
		ImGui::Unindent();
	}
}

void MinimapIcon::OnProcess(FW_FileProcessor& aProcessor)
{
	aProcessor.Process("MinimapIcon");
	aProcessor.Process(myIconSizeIndex);
	aProcessor.Process(IconColors[myIconColorIndex]);
	aProcessor.Process(IconShapes[myIconShapeIndex]);
}

void FilterColor::OnBuildUI()
{
	ImGui::ColorEdit4(myColorType, myColor, ImGuiColorEditFlags_NoInputs);
}

void FilterColor::OnProcess(FW_FileProcessor& aProcessor)
{
	FW_String text = "Set";
	text += myColorType;
	aProcessor.Process(text);

	int channel = static_cast<int>(myColor[0] * 255.f);
	aProcessor.Process(channel);

	channel = static_cast<int>(myColor[1] * 255.f);
	aProcessor.Process(channel);

	channel = static_cast<int>(myColor[2] * 255.f);
	aProcessor.Process(channel);

	channel = static_cast<int>(myColor[3] * 255.f);
	aProcessor.Process(channel);
}

void FilterBlock::BuildUI()
{
	myTextColor.BuildUI();
	myBorderColor.BuildUI();
	myBackgroundColor.BuildUI();
	myMinimapIcon.BuildUI();
}

void FilterBlock::Process(FW_FileProcessor& aProcessor)
{
	aProcessor.Process("#");
	aProcessor.Process(myTitle);
	aProcessor.AddNewline();

	if (myShow)
		aProcessor.Process("Show");
	else
		aProcessor.Process("Hide");

	aProcessor.AddNewline();

	myTextColor.Process(aProcessor);
	myBorderColor.Process(aProcessor);
	myBackgroundColor.Process(aProcessor);
	myMinimapIcon.Process(aProcessor);

	aProcessor.Process("SetFontSize");
	aProcessor.Process(myFontSize);
	aProcessor.AddNewline();
}

FilterMakerDockable::FilterMakerDockable()
	: Dockable(true)
{
}

void FilterMakerDockable::OnBuildUI()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File.."))
		{
			if (ImGui::Selectable("New"))
				ImGui::OpenPopup("New_Prefab_Popup");

			//if (ImGui::BeginPopup("New_Prefab_Popup"))
			//{
			//	ImGui::EndPopup();
			//}

			if (ImGui::Selectable("Save All"))
			{
				//for (EntityPrefab* prefab : prefabs)
				//	prefab->Save();
			}

			if (ImGui::BeginMenu("Open.."))
			{
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	myBlock.BuildUI();

	if (ImGui::Button("Save"))
	{
		FW_String filepath = "file.filter";
		FW_FileSystem::GetAbsoluteFilePath(filepath, filepath);

		FW_FileProcessor processor(filepath.GetBuffer(), FW_FileProcessor::WRITE);

		processor.Process("#Generated Filter");
		processor.AddNewline();
		processor.AddNewline();

		
		myBlock.Process(processor);

		//processor.Process("#Global Settings");
		//processor.AddNewline();
		//processor.Process("Show");
		//processor.AddNewline();
		//processor.Process("SetFontSize");
		//processor.Process(30);
		//processor.AddNewline();
		//processor.Process("Continue");
	}
}