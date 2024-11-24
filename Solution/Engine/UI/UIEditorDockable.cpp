#include "stdafx.h"
#include "UIEditorDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"

namespace Slush
{
	UIEditorDockable::UIEditorDockable(Slush::AssetStorage<Slush::UILayout>& aUILayoutStorage)
		: myUILayoutStorage(aUILayoutStorage)
	{
	}

	void UIEditorDockable::OnBuildUI()
	{
		Slush::UILayout* layout = myUILayoutStorage.GetAsset("StartGame");
		if (!layout)
		{
			SLUSH_ERROR("'StartGame' not found in UIEditorDockable");
			return;
		}

		if (ImGui::Button("Rebuild Layout"))
		{
			layout->myIsDirty = true;
		}

		ImGui::SameLine();

		if (ImGui::Button("Save Layout"))
		{
			layout->Save();
		}

		if (ImGui::Button("Add Button"))
			layout->myButtons.Add();

		if (ImGui::Button("Add Rect"))
			layout->myRects.Add();

		for (int i = 0; i < layout->myButtons.Count(); ++i)
		{
			ImGui::PushID(i);

			UILayout::Button& button = layout->myButtons[i];
			const char* label = button.myIdentifier.Empty() ? "Button" : button.myIdentifier.GetBuffer();

			FW_String realLabel = label;
			realLabel += "###";
			realLabel += i;
			if (ImGui::CollapsingHeader(realLabel.GetBuffer()))
			{
				ImGui::InputText("Identifier", &button.myIdentifier);
				
				ImGui::DragInt2("Position", &button.myPosition.x, 1, 0, 2560);
				ImGui::DragInt2("Size", &button.mySize.x, 1, 0, 2560);

				ImGui::InputText("Tet", &button.myText);

				ImGui::ColorEdit4("Color", button.myFloatColor);
				button.myColor = FW_Float_To_ARGB(button.myFloatColor[3], button.myFloatColor[0], button.myFloatColor[1], button.myFloatColor[2]);

				ImGui::ColorEdit4("Hover Color", button.myHoverFloatColor);
				button.myHoverColor = FW_Float_To_ARGB(button.myHoverFloatColor[3], button.myHoverFloatColor[0], button.myHoverFloatColor[1], button.myHoverFloatColor[2]);

				ImGui::ColorEdit4("Pressed Color", button.myPressedFloatColor);
				button.myPressedColor = FW_Float_To_ARGB(button.myPressedFloatColor[3], button.myPressedFloatColor[0], button.myPressedFloatColor[1], button.myPressedFloatColor[2]);

				if (ImGui::Button("Remove Button"))
				{
					layout->myButtons.RemoveCyclicAtIndex(i);
					--i;
					ImGui::PopID();
					continue;
				}
			}

			ImGui::PopID();
		}

		for (int i = 0; i < layout->myRects.Count(); ++i)
		{
			ImGui::PushID(i + layout->myButtons.Count());

			UILayout::Rect& rect = layout->myRects[i];
			//const char* label = button.myIdentifier.Empty() ? "Button" : button.myIdentifier.GetBuffer();
			const char* label = "Rect";

			FW_String realLabel = label;
			realLabel += "###";
			realLabel += i;
			if (ImGui::CollapsingHeader(realLabel.GetBuffer()))
			{
				//ImGui::InputText("Identifier", &button.myIdentifier);

				ImGui::DragInt2("Position", &rect.myPosition.x, 1, 0, 2560);
				ImGui::DragInt2("Size", &rect.mySize.x, 1, 0, 2560);

				ImGui::ColorEdit4("Color", rect.myFloatColor);
				rect.myColor = FW_Float_To_ARGB(rect.myFloatColor[3], rect.myFloatColor[0], rect.myFloatColor[1], rect.myFloatColor[2]);

				ImGui::ColorEdit4("Outline Color", rect.myFloatOutlineColor);
				rect.myOutlineColor = FW_Float_To_ARGB(rect.myFloatOutlineColor[3], rect.myFloatOutlineColor[0], rect.myFloatOutlineColor[1], rect.myFloatOutlineColor[2]);

				if (ImGui::Button("Remove Rect"))
				{
					layout->myRects.RemoveCyclicAtIndex(i);
					--i;
					ImGui::PopID();
					continue;
				}
			}

			ImGui::PopID();
		}
	}
}