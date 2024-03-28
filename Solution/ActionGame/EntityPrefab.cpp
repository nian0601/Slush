#include "EntityPrefab.h"
#include <imgui\imgui.h>
#include <FW_Math.h>
#include <FW_FileSystem.h>

void EntityPrefab::ComponentData::Parse(Slush::AssetParser::Handle aRootHandle)
{
	if (aRootHandle.IsReading())
	{
		Slush::AssetParser::Handle componentHandle = aRootHandle.ParseChildElement(myComponentName);
		if (componentHandle.IsValid())
		{
			myEnabled = true;
			OnParse(componentHandle);
		}
	}
	else
	{
		if (myEnabled)
		{
			Slush::AssetParser::Handle componentHandle = aRootHandle.ParseChildElement(myComponentName);
			if (componentHandle.IsValid())
				OnParse(componentHandle);
		}
	}
}

EntityPrefab::EntityPrefab(const char* aName)
	: myName(aName)
	, mySprite("sprite")
	, myAnimation("animation")
	, myProjectileShooting("projectileshooting")
	, myPlayerController("playercontroller")
	, myNPCController("npccontroller")
	, myHealth("health")
	, myPhysics("physics")
	, myRemoveOnCollision("removeoncollision")
	, myTargeting("targeting")
	, myWeaponComponent("weaponcomponent")
	, myExperience("experience")
	, myPickup("pickup")
{
}

void EntityPrefab::SaveToDisk()
{
	Slush::AssetParser parser;
	Slush::AssetParser::Handle rootHandle = parser.StartWriting("entityprefab");

	ParsePrefab(rootHandle);

	FW_String filepath = "Data/EntityPrefabs/";
	filepath += myName;
	filepath += ".prefab";
	parser.FinishWriting(filepath.GetBuffer());
}

void EntityPrefab::Load(const char* aFilePath, bool aIsAbsolutePath)
{
	Slush::AssetParser parser;
	Slush::AssetParser::Handle rootHandle = parser.Load(aFilePath);

	ParsePrefab(rootHandle);
}

void EntityPrefab::ParsePrefab(Slush::AssetParser::Handle aRootHandle)
{
	Slush::AssetParser::Handle entityTypeHandle = aRootHandle.ParseChildElement("entitytype");
	if (entityTypeHandle.IsValid())
	{
		int entityTypeAsInt = myEntityType;
		entityTypeHandle.ParseIntField("type", entityTypeAsInt);
		myEntityType = Entity::Type(entityTypeAsInt);
	}

	mySprite.Parse(aRootHandle);
	myAnimation.Parse(aRootHandle);
	myProjectileShooting.Parse(aRootHandle);
	myPlayerController.Parse(aRootHandle);
	myNPCController.Parse(aRootHandle);
	myHealth.Parse(aRootHandle);
	myPhysics.Parse(aRootHandle);
	myRemoveOnCollision.Parse(aRootHandle);
	myTargeting.Parse(aRootHandle);
	myWeaponComponent.Parse(aRootHandle);
	myExperience.Parse(aRootHandle);
	myPickup.Parse(aRootHandle);
}

void EntityPrefab::BuildUI()
{
	const char* entityTypeNames[] = { "Environment", "Player", "NPC", "Player Projectile", "NPC Projectile", "Pickup" };
	ImGui::Combo("Entity Type", &myEntityType, entityTypeNames, IM_ARRAYSIZE(entityTypeNames));

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

	if (BaseComponentUI(myTargeting.myEnabled, "Targeting", "Add Targeting"))
	{
		int type = myTargeting.myTargetType;
		ImGui::Combo("Target Type", &type, entityTypeNames, IM_ARRAYSIZE(entityTypeNames));
		myTargeting.myTargetType = Entity::Type(type);
		ImGui::TreePop();
	}

	if (BaseComponentUI(myWeaponComponent.myEnabled, "Weapon Component", "Add Weapon Component"))
	{
		ImGui::TreePop();
	}

	if (BaseComponentUI(myExperience.myEnabled, "Experience Component", "Add Experience Component"))
	{
		ImGui::TreePop();
	}

	if (BaseComponentUI(myPickup.myEnabled, "Pickup Component", "Add Pickup Component"))
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

void EntityPrefab::Sprite::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("radius", myRadius);
	aComponentHandle.ParseIntField("color", myColor);

	Slush::AssetParser::Handle sizeHandle = aComponentHandle.ParseChildElement("size");
	if (sizeHandle.IsValid())
	{
		sizeHandle.ParseFloatField("width", mySize.x);
		sizeHandle.ParseFloatField("height", mySize.y);
	}

	Slush::AssetParser::Handle colorHandle = aComponentHandle.ParseChildElement("floatColor");
	if (colorHandle.IsValid())
	{
		colorHandle.ParseFloatField("r", myFloatColor[0]);
		colorHandle.ParseFloatField("g", myFloatColor[1]);
		colorHandle.ParseFloatField("b", myFloatColor[2]);
		colorHandle.ParseFloatField("a", myFloatColor[3]);
	}
}

void EntityPrefab::ProjectileShooting::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("cooldown", myCooldown);
}

void EntityPrefab::Health::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseIntField("maxhealth", myMaxHealth);
}

void EntityPrefab::Physics::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseBoolField("isStatic", myStatic);
	aComponentHandle.ParseBoolField("matchSprite", myMatchSprite);

	aComponentHandle.ParseFloatField("radius", myRadius);

	Slush::AssetParser::Handle sizeHandle = aComponentHandle.ParseChildElement("size");
	if (sizeHandle.IsValid())
	{
		sizeHandle.ParseFloatField("width", mySize.x);
		sizeHandle.ParseFloatField("height", mySize.y);
	}
}

void EntityPrefab::Targeting::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	int targetTypeAsInt = myTargetType;
	aComponentHandle.ParseIntField("targettype", targetTypeAsInt);
	myTargetType = Entity::Type(targetTypeAsInt);
}
