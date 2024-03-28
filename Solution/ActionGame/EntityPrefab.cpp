#include "EntityPrefab.h"
#include <imgui\imgui.h>
#include <FW_Math.h>
#include <FW_FileSystem.h>

void EntityPrefab::ComponentData::Load(Slush::AssetParser::Handle aComponentHandle)
{
	if (!aComponentHandle.IsValid())
		return;

	myEnabled = true;
	OnLoad(aComponentHandle);
}

void EntityPrefab::ComponentData::Save(const char* aComponentName, Slush::AssetParser::Handle aRootHandle)
{
	if (!myEnabled)
		return;

	Slush::AssetParser::Handle componentHandle = aRootHandle.AddChildElement(aComponentName);
	OnSave(componentHandle);
}

EntityPrefab::EntityPrefab(const char* aName)
	: myName(aName)
{
}

void EntityPrefab::SaveToDisk()
{
	Slush::AssetParser parser;
	Slush::AssetParser::Handle rootHandle = parser.StartWriting("entityprefab");

	Slush::AssetParser::Handle entityTypeHandle = rootHandle.AddChildElement("entitytype");
	entityTypeHandle.WriteIntField("type", (int)myEntityType);

	mySprite.Save("sprite", rootHandle);
	myAnimation.Save("animation", rootHandle);
	myProjectileShooting.Save("projectileshooting", rootHandle);
	myPlayerController.Save("playercontroller", rootHandle);
	myNPCController.Save("npccontroller", rootHandle);
	myHealth.Save("health", rootHandle);
	myPhysics.Save("physics", rootHandle);
	myRemoveOnCollision.Save("removeoncollision", rootHandle);
	myTargeting.Save("targeting", rootHandle);
	myWeaponComponent.Save("weaponcomponent", rootHandle);
	myExperience.Save("experience", rootHandle);
	myPickup.Save("pickup", rootHandle);

	FW_String filepath = "Data/EntityPrefabs/";
	filepath += myName;
	filepath += ".prefab";
	parser.FinishWriting(filepath.GetBuffer());
}

void EntityPrefab::Load(const char* aFilePath, bool aIsAbsolutePath)
{
	Slush::AssetParser parser;
	Slush::AssetParser::Handle rootHandle = parser.Load(aFilePath);

	Slush::AssetParser::Handle entityTypeHandle = rootHandle.GetChildElement("entitytype");
	myEntityType = entityTypeHandle.GetIntField("type");

	mySprite.Load(rootHandle.GetChildElement("sprite"));
	myAnimation.Load(rootHandle.GetChildElement("animation"));
	myProjectileShooting.Load(rootHandle.GetChildElement("projectileshooting"));
	myPlayerController.Load(rootHandle.GetChildElement("playercontroller"));
	myNPCController.Load(rootHandle.GetChildElement("npccontroller"));
	myHealth.Load(rootHandle.GetChildElement("health"));
	myPhysics.Load(rootHandle.GetChildElement("physics"));
	myRemoveOnCollision.Load(rootHandle.GetChildElement("removeoncollision"));
	myTargeting.Load(rootHandle.GetChildElement("targeting"));
	myWeaponComponent.Load(rootHandle.GetChildElement("weaponcomponent"));
	myExperience.Load(rootHandle.GetChildElement("experience"));
	myPickup.Load(rootHandle.GetChildElement("pickup"));
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

void EntityPrefab::Sprite::OnSave(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.WriteFloatField("radius", myRadius);
	aComponentHandle.WriteIntField("color", myColor);

	Slush::AssetParser::Handle sizeHandle = aComponentHandle.AddChildElement("size");
	sizeHandle.WriteFloatField("width", mySize.x);
	sizeHandle.WriteFloatField("height", mySize.y);

	Slush::AssetParser::Handle colorHandle = aComponentHandle.AddChildElement("floatColor");
	colorHandle.WriteFloatField("r", myFloatColor[0]);
	colorHandle.WriteFloatField("g", myFloatColor[1]);
	colorHandle.WriteFloatField("b", myFloatColor[2]);
	colorHandle.WriteFloatField("a", myFloatColor[3]);
}

void EntityPrefab::Sprite::OnLoad(Slush::AssetParser::Handle aComponentHandle)
{
	myRadius = aComponentHandle.GetFloatField("radius");
	myColor = aComponentHandle.GetIntField("color");

	Slush::AssetParser::Handle sizeHandle = aComponentHandle.GetChildElement("size");
	if (sizeHandle.IsValid())
	{
		mySize.x = sizeHandle.GetFloatField("width");
		mySize.y = sizeHandle.GetFloatField("height");
	}

	Slush::AssetParser::Handle colorHandle = aComponentHandle.GetChildElement("floatColor");
	if (colorHandle.IsValid())
	{
		myFloatColor[0] = colorHandle.GetFloatField("r");
		myFloatColor[1] = colorHandle.GetFloatField("g");
		myFloatColor[2] = colorHandle.GetFloatField("b");
		myFloatColor[3] = colorHandle.GetFloatField("a");
	}
}

void EntityPrefab::ProjectileShooting::OnSave(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.WriteFloatField("cooldown", myCooldown);
}

void EntityPrefab::ProjectileShooting::OnLoad(Slush::AssetParser::Handle aComponentHandle)
{
	myCooldown = aComponentHandle.GetFloatField("cooldown");
}

void EntityPrefab::Health::OnSave(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.WriteIntField("maxhealth", myMaxHealth);
}

void EntityPrefab::Health::OnLoad(Slush::AssetParser::Handle aComponentHandle)
{
	myMaxHealth = aComponentHandle.GetIntField("maxhealth");
}

void EntityPrefab::Physics::OnSave(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.WriteBoolField("isStatic", myStatic);
	aComponentHandle.WriteBoolField("matchSprite", myMatchSprite);

	aComponentHandle.WriteFloatField("radius", myRadius);

	Slush::AssetParser::Handle sizeHandle = aComponentHandle.AddChildElement("size");
	sizeHandle.WriteFloatField("width", mySize.x);
	sizeHandle.WriteFloatField("height", mySize.y);
}

void EntityPrefab::Physics::OnLoad(Slush::AssetParser::Handle aComponentHandle)
{
	myStatic = aComponentHandle.GetBoolField("isStatic");
	myMatchSprite = aComponentHandle.GetBoolField("matchSprite");

	myRadius = aComponentHandle.GetFloatField("radius");

	Slush::AssetParser::Handle sizeHandle = aComponentHandle.GetChildElement("size");
	if (sizeHandle.IsValid())
	{
		mySize.x = sizeHandle.GetFloatField("width");
		mySize.y = sizeHandle.GetFloatField("height");
	}
}

void EntityPrefab::Targeting::OnSave(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.WriteIntField("targettype", (int)myTargetType);
}

void EntityPrefab::Targeting::OnLoad(Slush::AssetParser::Handle aComponentHandle)
{
	myTargetType = Entity::Type(aComponentHandle.GetIntField("targettype"));
}