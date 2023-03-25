
#include "Dockable_CardEditor.h"

#include <imgui/imgui.h>
#include <imgui/imgui-SFML.h>

#include <Core/Engine.h>
#include "FW_FileProcessor.h"

Dockable_CardEditor::Dockable_CardEditor()
	: myFileName("")
	, myTitle("")
	, myDescription("")
	, myTextureName("")
{
}

void Dockable_CardEditor::BuildUI()
{
	if (ImGui::Begin("Card Editor"))
	{
		const char* cardTypeNames[] = { "NONE", "ROOM", "BOSS", "HERO" };
		ImGui::Combo("Card Type", &myCardType, cardTypeNames, IM_ARRAYSIZE(cardTypeNames));

		if (myCardType != NONE)
		{
			ImGui::Separator();

			ImGui::InputText("Title", &myTitle);
			ImGui::InputText("Description", &myDescription);
			ImGui::InputText("Texture", &myTextureName);

			ImGui::Separator();

			if (myCardType == ROOM)
			{
				ImGui::InputInt("Damage", &myDamageValue);

				const char* treasureTypeNames[] = { "CLERIC", "FIGHTER", "MAGE", "THEIF" };
				ImGui::Combo("Treasure Type", &myTreasureType, treasureTypeNames, IM_ARRAYSIZE(treasureTypeNames));
			}
			else if (myCardType == BOSS)
			{
				ImGui::InputInt("XP", &myXPValue);
			}
			else if (myCardType == HERO)
			{
				ImGui::InputInt("Health", &myHealthValue);
				ImGui::InputInt("Wound", &myWoundValue);
				ImGui::InputInt("Soul", &mySoulValue);
			}

			ImGui::Separator();

			ImGui::InputText("Asset Name", &myFileName, 64);
			if (ImGui::Button("Save"))
			{
				FW_String filepath = Slush::Engine::GetInstance().GetDataFolder();
				filepath += "Data/";
				filepath += myFileName;
				switch (myCardType)
				{
				case ROOM:
					filepath += ".room"; break;
				case BOSS:
					filepath += ".boss"; break;
				case HERO:
					filepath += ".hero"; break;
				default:
					break;
				}

				FW_FileProcessor processor(filepath.GetBuffer(), FW_FileProcessor::WRITE);
				processor.Process("#version");
				processor.Process("1.0.0");
				processor.AddNewline();

				processor.Process("#title");
				processor.Process(myTitle);
				processor.AddNewline();

				processor.Process("#description");
				processor.Process(myDescription);
				processor.AddNewline();

				processor.Process("#texture");
				processor.Process(myTextureName);
				processor.AddNewline();
			}

			if (ImGui::Button("Test loading"))
			{
				FW_String filepath = Slush::Engine::GetInstance().GetDataFolder();
				filepath += "Data/";
				filepath += myFileName;
				switch (myCardType)
				{
				case ROOM:
					filepath += ".room"; break;
				case BOSS:
					filepath += ".boss"; break;
				case HERO:
					filepath += ".hero"; break;
				default:
					break;
				}

				FW_FileProcessor processor(filepath.GetBuffer(), FW_FileProcessor::READ);

				FW_String versionHeader;
				FW_String versionNumber;
				processor.Process(versionHeader);
				processor.Process(versionNumber);

				FW_String dummy;
				processor.Process(dummy); //#title
				//FW_String title;
				processor.ReadRestOfLine(myTitle);

				processor.Process(dummy); //#description
				//FW_String description;
				processor.ReadRestOfLine(myDescription);

				processor.Process(dummy); //#texture
				//FW_String texture;
				processor.ReadRestOfLine(myTextureName);
			}

		}
	}
	ImGui::End();
}

