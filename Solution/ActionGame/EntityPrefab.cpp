#include "EntityPrefab.h"
#include <imgui\imgui.h>
#include <FW_Math.h>
#include <FW_FileSystem.h>

EntityPrefab::EntityPrefab(const char* aName)
	: myName(aName)
{
}

void EntityPrefab::SaveToDisk()
{
	FW_String filepath = "Data/EntityPrefabs/";
	filepath += myName;
	filepath += ".prefab";

	FW_FileSystem::GetAbsoluteFilePath(filepath, filepath);

	FW_FileProcessor processor(filepath.GetBuffer(), FW_FileProcessor::WRITE);

	int entityType = myEntityType;
	processor.Process("#entitytype");
	processor.Process(entityType);
	processor.AddNewline();

	mySprite.SaveToDisk(processor);
	myAnimation.SaveToDisk(processor);
	myProjectileShooting.SaveToDisk(processor);
	myPlayerController.SaveToDisk(processor);
	myNPCController.SaveToDisk(processor);
	myHealth.SaveToDisk(processor);
	myPhysics.SaveToDisk(processor);
	myRemoveOnCollision.SaveToDisk(processor);
}

void EntityPrefab::Load(const char* aFilePath, bool aIsAbsolutePath)
{
	FW_FileParser parser(aFilePath);

	FW_String line;
	FW_String fieldName;
	while (parser.ReadLine(line))
	{
		parser.TrimBeginAndEnd(line);
		fieldName = parser.TakeFirstWord(line);

		if (fieldName == "#entitytype")
		{
			int type = parser.GetInt(parser.TakeFirstWord(line));
			myEntityType = Entity::Type(type);
		}
		else if (fieldName == "#sprite")
		{
			mySprite.myEnabled = true;
			mySprite.LoadFromDisk(parser);
		}
		else if (fieldName == "#animation")
		{
			myAnimation.myEnabled = true;
			LoadEmptyComponent(parser);
		}
		else if (fieldName == "#projectileshooting")
		{
			myProjectileShooting.myEnabled = true;
			myProjectileShooting.LoadFromDisk(parser);
		}
		else if (fieldName == "#playercontroller")
		{
			myPlayerController.myEnabled = true;
			LoadEmptyComponent(parser);
		}
		else if (fieldName == "#npccontroller")
		{
			myNPCController.myEnabled = true;
			LoadEmptyComponent(parser);
		}
		else if (fieldName == "#health")
		{
			myHealth.myEnabled = true;
			myHealth.LoadFromDisk(parser);
		}
		else if (fieldName == "#physics")
		{
			myPhysics.myEnabled = true;
			myPhysics.LoadFromDisk(parser);
		}
		else if (fieldName == "#removeoncollision")
		{
			myRemoveOnCollision.myEnabled = true;
			LoadEmptyComponent(parser);
		}
	}
}

void EntityPrefab::LoadEmptyComponent(FW_FileParser& aParser)
{
	FW_String line;
	FW_String fieldName;
	while (aParser.ReadLine(line))
	{
		aParser.TrimBeginAndEnd(line);
		fieldName = aParser.TakeFirstWord(line);

		if (fieldName == "#end")
			return;
	}
}

void EntityPrefab::BuildUI()
{
	if (BaseComponentUI(mySprite.myEnabled, "Sprite", "Add Sprite"))
	{
		ImGui::ColorEdit4("Color", mySprite.myFloatColor);
		ImGui::InputFloat("Radius", &mySprite.myRadius, 1.f, 10.f, "%.2f");
		ImGui::InputFloat2("Size", &mySprite.mySize.x, "%.2f");

		mySprite.myColor = FW_Float_To_ARGB(mySprite.myFloatColor[3], mySprite.myFloatColor[0], mySprite.myFloatColor[1], mySprite.myFloatColor[2]);

		ImGui::TreePop();
	}

	if (BaseComponentUI(myAnimation.myEnabled, "Animation", "Add Animation"))
	{
		ImGui::TreePop();
	}

	if (BaseComponentUI(myProjectileShooting.myEnabled, "Projectile Shooter", "Add Projectile Shooter"))
	{
		ImGui::InputFloat("Cooldown", &myProjectileShooting.myCooldown, 0.1f, 1.f, "%.2f");
		ImGui::TreePop();
	}

	if (BaseComponentUI(myPlayerController.myEnabled, "Player Controller", "Add Player Controller"))
	{
		ImGui::TreePop();
	}

	if (BaseComponentUI(myNPCController.myEnabled, "NPC Controller", "Add NPC Controller"))
	{
		ImGui::TreePop();
	}

	if (BaseComponentUI(myHealth.myEnabled, "Health", "Add Health"))
	{
		ImGui::InputInt("Max Health", &myHealth.myMaxHealth);
		ImGui::TreePop();
	}

	if (BaseComponentUI(myPhysics.myEnabled, "Physics", "Add Physics"))
	{
		ImGui::Checkbox("Is Static", &myPhysics.myStatic);
		ImGui::Checkbox("Match Sprite", &myPhysics.myMatchSprite);

		ImGui::InputFloat("Radius", &myPhysics.myRadius, 1.f, 10.f, "%.2f");
		ImGui::InputFloat2("Size", &myPhysics.mySize.x, "%.2f");

		ImGui::TreePop();
	}

	if (BaseComponentUI(myRemoveOnCollision.myEnabled, "Remove On Collision", "Add Remove On Collision"))
	{
		ImGui::TreePop();
	}
}

bool EntityPrefab::BaseComponentUI(bool& aEnabledFlag, const char* aComponentLabel, const char* aAddComponentLabel)
{
	if (aEnabledFlag)
	{
		if (ImGui::TreeNode(aComponentLabel))
		{
			if (ImGui::Button("Remove"))
				aEnabledFlag = false;

			return true;
		}
	}
	else
	{
		aEnabledFlag = ImGui::Button(aAddComponentLabel);
	}

	return false;
}

void EntityPrefab::Sprite::SaveToDisk(FW_FileProcessor& aProcessor)
{
	if (myEnabled)
	{
		aProcessor.Process("#sprite");
		aProcessor.AddNewline();

		aProcessor.Process("radius");
		aProcessor.Process(myRadius);
		aProcessor.AddNewline();

		aProcessor.Process("size");
		aProcessor.Process(mySize.x);
		aProcessor.Process(mySize.y);
		aProcessor.AddNewline();

		aProcessor.Process("color");
		aProcessor.Process(myColor);
		aProcessor.AddNewline();

		aProcessor.Process("floatColor");
		aProcessor.Process(myFloatColor[0]);
		aProcessor.Process(myFloatColor[1]);
		aProcessor.Process(myFloatColor[2]);
		aProcessor.Process(myFloatColor[3]);
		aProcessor.AddNewline();

		aProcessor.Process("#end");
		aProcessor.AddNewline();
	}
}

void EntityPrefab::Sprite::LoadFromDisk(FW_FileParser& aParser)
{
	FW_String line;
	FW_String fieldName;
	while (aParser.ReadLine(line))
	{
		aParser.TrimBeginAndEnd(line);
		fieldName = aParser.TakeFirstWord(line);

		if (fieldName == "#end")
			return;

		if (fieldName == "radius")
		{
			myRadius = aParser.GetFloat(aParser.TakeFirstWord(line));
		}
		else if (fieldName == "size")
		{
			mySize.x = aParser.GetFloat(aParser.TakeFirstWord(line));
			mySize.y = aParser.GetFloat(aParser.TakeFirstWord(line));
		}
		else if (fieldName == "color")
		{
			myColor = aParser.GetInt(aParser.TakeFirstWord(line));
		}
		else if (fieldName == "floatColor")
		{
			myFloatColor[0] = aParser.GetFloat(aParser.TakeFirstWord(line));
			myFloatColor[1] = aParser.GetFloat(aParser.TakeFirstWord(line));
			myFloatColor[2] = aParser.GetFloat(aParser.TakeFirstWord(line));
			myFloatColor[3] = aParser.GetFloat(aParser.TakeFirstWord(line));
		}
	}
}

void EntityPrefab::Animation::SaveToDisk(FW_FileProcessor& aProcessor)
{
	if (myEnabled)
	{
		aProcessor.Process("#animation");
		aProcessor.AddNewline();

		aProcessor.Process("#end");
		aProcessor.AddNewline();
	}
}

void EntityPrefab::ProjectileShooting::SaveToDisk(FW_FileProcessor& aProcessor)
{
	if (myEnabled)
	{
		aProcessor.Process("#projectileshooting");
		aProcessor.AddNewline();

		aProcessor.Process("cooldown");
		aProcessor.Process(myCooldown);
		aProcessor.AddNewline();

		aProcessor.Process("#end");
		aProcessor.AddNewline();
	}
}

void EntityPrefab::ProjectileShooting::LoadFromDisk(FW_FileParser& aParser)
{
	FW_String line;
	FW_String fieldName;
	while (aParser.ReadLine(line))
	{
		aParser.TrimBeginAndEnd(line);
		fieldName = aParser.TakeFirstWord(line);

		if (fieldName == "#end")
			return;

		if (fieldName == "cooldown")
		{
			myCooldown = aParser.GetFloat(aParser.TakeFirstWord(line));
		}
	}
}

void EntityPrefab::PlayerController::SaveToDisk(FW_FileProcessor& aProcessor)
{
	if (myEnabled)
	{
		aProcessor.Process("#playercontroller");
		aProcessor.AddNewline();

		aProcessor.Process("#end");
		aProcessor.AddNewline();
	}
}

void EntityPrefab::NPCController::SaveToDisk(FW_FileProcessor& aProcessor)
{
	if (myEnabled)
	{
		aProcessor.Process("#npccontroller");
		aProcessor.AddNewline();

		aProcessor.Process("#end");
		aProcessor.AddNewline();
	}
}

void EntityPrefab::Health::SaveToDisk(FW_FileProcessor& aProcessor)
{
	if (myEnabled)
	{
		aProcessor.Process("#health");
		aProcessor.AddNewline();

		aProcessor.Process("maxhealth");
		aProcessor.Process(myMaxHealth);
		aProcessor.AddNewline();

		aProcessor.Process("#end");
		aProcessor.AddNewline();
	}
}

void EntityPrefab::Health::LoadFromDisk(FW_FileParser& aParser)
{
	FW_String line;
	FW_String fieldName;
	while (aParser.ReadLine(line))
	{
		aParser.TrimBeginAndEnd(line);
		fieldName = aParser.TakeFirstWord(line);

		if (fieldName == "#end")
			return;

		if (fieldName == "maxhealth")
		{
			myMaxHealth = aParser.GetInt(aParser.TakeFirstWord(line));
		}
	}
}

void EntityPrefab::Physics::SaveToDisk(FW_FileProcessor& aProcessor)
{
	if (myEnabled)
	{
		aProcessor.Process("#physics");
		aProcessor.AddNewline();

		aProcessor.Process("isStatic");
		aProcessor.Process(myStatic);
		aProcessor.AddNewline();

		aProcessor.Process("matchsprite");
		aProcessor.Process(myMatchSprite);
		aProcessor.AddNewline();

		aProcessor.Process("radius");
		aProcessor.Process(myRadius);
		aProcessor.AddNewline();

		aProcessor.Process("size");
		aProcessor.Process(mySize.x);
		aProcessor.Process(mySize.y);
		aProcessor.AddNewline();

		aProcessor.Process("#end");
		aProcessor.AddNewline();
	}
}

void EntityPrefab::Physics::LoadFromDisk(FW_FileParser& aParser)
{
	FW_String line;
	FW_String fieldName;
	while (aParser.ReadLine(line))
	{
		aParser.TrimBeginAndEnd(line);
		fieldName = aParser.TakeFirstWord(line);

		if (fieldName == "#end")
			return;

		if (fieldName == "isStatic")
		{
			myStatic = aParser.GetInt(aParser.TakeFirstWord(line)) == 1;
		}
		else if (fieldName == "matchsprite")
		{
			myMatchSprite = aParser.GetInt(aParser.TakeFirstWord(line)) == 1;
		}
		else if (fieldName == "radius")
		{
			myRadius = aParser.GetFloat(aParser.TakeFirstWord(line));
		}
		else if (fieldName == "size")
		{
			mySize.x = aParser.GetFloat(aParser.TakeFirstWord(line));
			mySize.y = aParser.GetFloat(aParser.TakeFirstWord(line));
		}
	}
}

void EntityPrefab::RemoveOnCollision::SaveToDisk(FW_FileProcessor& aProcessor)
{
	if (myEnabled)
	{
		aProcessor.Process("#removeoncollision");
		aProcessor.AddNewline();

		aProcessor.Process("#end");
		aProcessor.AddNewline();
	}
}