#include "stdafx.h"

#include "EntityPrefab.h"

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
	: DataAsset(aName)
	, myName(aName)
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
	, myStats("stats")
	, myDamageDealer("damagedealer")
	, myHealthBar("healthbar")
{
}

void EntityPrefab::OnParse(Slush::AssetParser::Handle aRootHandle)
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
	myStats.Parse(aRootHandle);
	myDamageDealer.Parse(aRootHandle);
	myHealthBar.Parse(aRootHandle);
}

void EntityPrefab::BuildUI()
{
	ImGui::SeparatorText(myName.GetBuffer());

	const char* entityTypeNames[] = { "Environment", "Player", "NPC", "Player Projectile", "NPC Projectile", "Pickup" };
	ImGui::Combo("Entity Type", &myEntityType, entityTypeNames, IM_ARRAYSIZE(entityTypeNames));

	FW_GrowingArray<MissingComponent> missingComponents;
	if (BaseComponentUI(mySprite.myEnabled, "Sprite", missingComponents))
	{
		ImGui::ColorEdit4("Color", mySprite.myFloatColor);
		ImGui::InputFloat("Radius", &mySprite.myRadius, 1.f, 10.f, "%.2f");
		ImGui::InputFloat2("Size", &mySprite.mySize.x, "%.2f");

		mySprite.myColor = FW_Float_To_ARGB(mySprite.myFloatColor[3], mySprite.myFloatColor[0], mySprite.myFloatColor[1], mySprite.myFloatColor[2]);

		ImGui::Unindent();
	}

	if (BaseComponentUI(myAnimation.myEnabled, "Animation", missingComponents))
	{
		ImGui::Unindent();
	}

	if (BaseComponentUI(myProjectileShooting.myEnabled, "Projectile Shooter", missingComponents))
	{
		ImGui::SetNextItemWidth(100.f);
		ImGui::InputFloat("Cooldown", &myProjectileShooting.myCooldown, 0.1f, 1.f, "%.2f");

		ImGui::SetNextItemWidth(100.f);
		ImGui::InputFloat("Projectile Speed", &myProjectileShooting.myProjectileSpeed, 1.f, 10.f, "%.2f");

		ImGui::SetNextItemWidth(100.f);
		ImGui::InputFloat("Projectile Spawn Offset", &myProjectileShooting.myProjectileSpawnOffset, 0.1f, 1.f, "%.2f");

		ImGui::Unindent();
	}

	if (BaseComponentUI(myPlayerController.myEnabled, "Player Controller", missingComponents))
	{
		ImGui::Unindent();
	}

	if (BaseComponentUI(myNPCController.myEnabled, "NPC Controller", missingComponents))
	{
		ImGui::Unindent();
	}

	if (BaseComponentUI(myHealth.myEnabled, "Health", missingComponents))
	{
		ImGui::InputInt("Max Health", &myHealth.myMaxHealth);
		ImGui::InputFloat("Grace Period Duration", &myHealth.myGracePeriodDuration);

		ImGui::Unindent();
	}

	if (BaseComponentUI(myPhysics.myEnabled, "Physics", missingComponents))
	{
		ImGui::Checkbox("Is Static", &myPhysics.myStatic);
		ImGui::Checkbox("Is Sensor", &myPhysics.mySensor);
		ImGui::Checkbox("Match Sprite", &myPhysics.myMatchSprite);

		ImGui::InputFloat("Radius", &myPhysics.myRadius, 1.f, 10.f, "%.2f");
		ImGui::InputFloat2("Size", &myPhysics.mySize.x, "%.2f");

		ImGui::Unindent();
	}

	if (BaseComponentUI(myRemoveOnCollision.myEnabled, "Remove On Collision", missingComponents))
	{
		ImGui::Unindent();
	}

	if (BaseComponentUI(myTargeting.myEnabled, "Targeting", missingComponents))
	{
		int type = myTargeting.myTargetType;
		ImGui::Combo("Target Type", &type, entityTypeNames, IM_ARRAYSIZE(entityTypeNames));
		myTargeting.myTargetType = Entity::Type(type);

		ImGui::Unindent();
	}

	if (BaseComponentUI(myWeaponComponent.myEnabled, "Weapon", missingComponents))
	{
		ImGui::SetNextItemWidth(100.f);
		ImGui::InputFloat("Base Cooldown", &myWeaponComponent.myBaseCooldown, 0.05f, 0.1f, "%.2f");

		ImGui::SetNextItemWidth(100.f);
		ImGui::InputFloat("Base Projectile Speed", &myWeaponComponent.myBaseProjectileSpeed, 1.f, 100.f, "%.2f");

		ImGui::SetNextItemWidth(100.f);
		ImGui::InputInt("Base Damage", &myWeaponComponent.myBaseDamage);

		ImGui::Unindent();
	}

	if (BaseComponentUI(myExperience.myEnabled, "Experience", missingComponents))
	{
		ImGui::Unindent();
	}

	if (BaseComponentUI(myPickup.myEnabled, "Pickup", missingComponents))
	{
		ImGui::Unindent();
	}

	if (BaseComponentUI(myStats.myEnabled, "Stats", missingComponents))
	{
		ImGui::SetNextItemWidth(100.f);
		ImGui::InputInt("Max Cooldown Upgrades", &myStats.myMaxCooldownUpgrades);

		ImGui::SetNextItemWidth(100.f);
		ImGui::InputFloat("Cooldown Per Upgrade", &myStats.myCooldownPerUpgrade, 0.05f, 1.f, "%.2f");

		ImGui::SetNextItemWidth(100.f);
		ImGui::InputInt("Max Damage Upgrades", &myStats.myMaxDamageUpgrades);

		ImGui::SetNextItemWidth(100.f);
		ImGui::InputFloat("Damage Per Upgrade", &myStats.myDamagePerUpgrade, 0.05f, 1.f, "%.2f");

		ImGui::SetNextItemWidth(100.f);
		ImGui::InputInt("Max Additional Projectiles Upgrades", &myStats.myMaxAdditionalProjectileUpgrades);

		ImGui::SetNextItemWidth(100.f);
		ImGui::InputFloat("Additional Projectiles Per Upgrade", &myStats.myAdditionalProjectilesPerUpgrade, 1.f, 1.f, "%.0f");

		ImGui::Unindent();
	}

	if (BaseComponentUI(myDamageDealer.myEnabled, "DamageDealer", missingComponents))
	{
		ImGui::SetNextItemWidth(100.f);
		ImGui::InputInt("Damage", &myDamageDealer.myDamage);

		ImGui::Unindent();
	}

	if (BaseComponentUI(myHealthBar.myEnabled, "HealthBar", missingComponents))
	{
		ImGui::Unindent();
	}

	BuildMissingComponentsUI(missingComponents);
}

bool EntityPrefab::BaseComponentUI(bool& aEnabledFlag, const char* aComponentLabel, FW_GrowingArray<MissingComponent>& someMissingComponentsOut)
{
	FW_String label = aComponentLabel;
	label += " Component";

	if (aEnabledFlag)
	{
		if (ImGui::CollapsingHeader(label.GetBuffer(), &aEnabledFlag))
		{
			ImGui::Indent();
			return true;
		}
	}
	else
	{	
		someMissingComponentsOut.Add({ label, &aEnabledFlag });
	}

	return false;
}

void EntityPrefab::BuildMissingComponentsUI(const FW_GrowingArray<MissingComponent>& someMissingComponents)
{
	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("add_component_popup");

	if (ImGui::BeginPopup("add_component_popup"))
	{
		for (int i = 0; i < someMissingComponents.Count(); ++i)
			if (ImGui::Selectable(someMissingComponents[i].myLabel.GetBuffer()))
				*someMissingComponents[i].myEnabledFlag = true;

		ImGui::EndPopup();
	}
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
	aComponentHandle.ParseFloatField("projectilespeed", myProjectileSpeed);
	aComponentHandle.ParseFloatField("projectilespawnoffset", myProjectileSpawnOffset);
}

void EntityPrefab::Health::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseIntField("maxhealth", myMaxHealth);
	aComponentHandle.ParseFloatField("graceperiodduration", myGracePeriodDuration);
}

void EntityPrefab::Physics::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseBoolField("isStatic", myStatic);
	aComponentHandle.ParseBoolField("isSensor", mySensor);
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

void EntityPrefab::StatsComponent::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseIntField("maxcooldownupgrades", myMaxCooldownUpgrades);
	aComponentHandle.ParseFloatField("cooldownperupgrade", myCooldownPerUpgrade);

	aComponentHandle.ParseIntField("maxdamageupgrades", myMaxDamageUpgrades);
	aComponentHandle.ParseFloatField("damageperupgrade", myDamagePerUpgrade);

	aComponentHandle.ParseIntField("maxadditionalprojectileupgrades", myMaxAdditionalProjectileUpgrades);
	aComponentHandle.ParseFloatField("additionalprojectilesperupgrade", myAdditionalProjectilesPerUpgrade);
}

void EntityPrefab::DamageDealer::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseIntField("damage", myDamage);
}

void EntityPrefab::Weapon::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("basecooldown", myBaseCooldown);
	aComponentHandle.ParseFloatField("baseprojectilespeed", myBaseProjectileSpeed);
	aComponentHandle.ParseIntField("basedamage", myBaseDamage);
}
