
#include "Dockable_CardEditor.h"

#include <imgui/imgui.h>
#include <imgui/imgui-SFML.h>

#include <Core/Engine.h>
#include "FW_FileProcessor.h"
#include <Core/AssetStorage.h>
#include <Graphics/Texture.h>

Dockable_CardEditor::Dockable_CardEditor()
	: myFileName("")
	, myTitle("")
	, myDescription("")
	, myTextureName("")
{
	myCardsFilePath = Slush::Engine::GetInstance().GetDataFolder();
	myCardsFilePath += "Data/Cards";
}

void Dockable_CardEditor::BuildUI(const Slush::AssetStorage<Slush::Texture>& someTextures)
{
	if (ImGui::Begin("Card Editor"))
	{
		if(ImGui::Button("Load.."))
			ImGui::OpenPopup("load_card_popup");

		if (ImGui::BeginPopup("load_card_popup"))
		{
			FW_GrowingArray<FW_FileSystem::FileInfo> cardFiles;
			FW_FileSystem::GetAllFilesFromDirectory(myCardsFilePath.GetBuffer(), cardFiles);

			ImGui::Text("Cards");
			ImGui::Separator();

			for (const FW_FileSystem::FileInfo& cardInfo : cardFiles)
			{
				if (ImGui::Selectable(cardInfo.myFileNameNoExtention.GetBuffer()))
				{
					LoadCard(cardInfo.myFilePath);
					break;
				}
			}

			ImGui::EndPopup();
		}

		ImGui::Separator();
		
		const char* cardTypeNames[] = { "NONE", "ROOM", "BOSS", "HERO" };
		ImGui::Combo("Card Type", &myCardType, cardTypeNames, IM_ARRAYSIZE(cardTypeNames));

		if (myCardType != NONE)
		{
			ImGui::Separator();

			ImGui::InputText("Title", &myTitle);
			ImGui::InputText("Description", &myDescription);
			ImGui::InputText("Texture", &myTextureName);
			ImGui::SameLine();
			if (ImGui::Button("..."))
				ImGui::OpenPopup("select_texture_popup");

			if (ImGui::BeginPopup("select_texture_popup"))
			{
				ImGui::Text("Textures");
				ImGui::Separator();
				
				//if(!ImGui::IsAnyItemActive())
				//	ImGui::SetKeyboardFocusHere();

				static ImGuiTextFilter textureFilter;
				textureFilter.Draw("Search");

				const FW_GrowingArray<Slush::Texture*> textures = someTextures.GetAllAssets();
				for (const Slush::Texture* texture : textures)
				{
					if(!textureFilter.PassFilter(texture->GetAssetName().GetBuffer()))
						continue;

					if (ImGui::Selectable(texture->GetAssetName().GetBuffer()))
					{
						myTextureName = texture->GetAssetName();
						break;
					}
				}

				ImGui::EndPopup();
			}

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
				SaveCard();
			}
		}
	}
	ImGui::End();
}

void Dockable_CardEditor::SaveCard()
{
	FW_String filepath = myCardsFilePath;
	filepath += "/";
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

	if (myCardType == ROOM)
	{
		processor.Process("#damage");
		processor.Process(myDamageValue);
		processor.AddNewline();

		const char* treasureTypeNames[] = { "CLERIC", "FIGHTER", "MAGE", "THEIF" };
		processor.Process("#treasure");
		processor.Process(treasureTypeNames[myTreasureType]);
		processor.AddNewline();
	}
	else if (myCardType == BOSS)
	{
		processor.Process("#xp");
		processor.Process(myXPValue);
		processor.AddNewline();
	}
	else if (myCardType == HERO)
	{
		processor.Process("#health");
		processor.Process(myHealthValue);
		processor.AddNewline();

		processor.Process("#wound");
		processor.Process(myWoundValue);
		processor.AddNewline();

		processor.Process("#soul");
		processor.Process(mySoulValue);
		processor.AddNewline();
	}
}

void Dockable_CardEditor::LoadCard(const FW_String& aFilePath)
{
	FW_FileSystem::GetFileNameNoExtention(aFilePath, myFileName);

	FW_String fileExtention;
	FW_FileSystem::GetFileExtention(aFilePath, fileExtention);
	if (fileExtention == "room")
		myCardType = ROOM;
	else if (fileExtention == "boss")
		myCardType = BOSS;
	else if (fileExtention == "hero")
		myCardType = HERO;
	else if (fileExtention == "spell")
		myCardType = SPELL;

	FW_FileProcessor processor(aFilePath.GetBuffer(), FW_FileProcessor::READ);

	FW_String versionHeader;
	FW_String versionNumber;
	processor.Process(versionHeader);
	processor.Process(versionNumber);

	FW_String dummy;
	processor.Process(dummy); //#title
	processor.ReadRestOfLine(myTitle);

	processor.Process(dummy); //#description
	processor.ReadRestOfLine(myDescription);

	processor.Process(dummy); //#texture
	processor.ReadRestOfLine(myTextureName);

	if (myCardType == ROOM)
	{
		processor.Process(dummy); //#damage
		processor.Process(myDamageValue);

		processor.Process(dummy); //#treasure
		processor.Process(myTreasureType);
	}
	else if (myCardType == BOSS)
	{
		processor.Process(dummy); //#xp
		processor.Process(myXPValue);
	}
	else if (myCardType == HERO)
	{
		processor.Process(dummy); //#health
		processor.Process(myHealthValue);

		processor.Process(dummy); //#wound
		processor.Process(myWoundValue);

		processor.Process(dummy); //#soul
		processor.Process(mySoulValue);
	}
}

