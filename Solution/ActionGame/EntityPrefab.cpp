#include "stdafx.h"

#include "EntityPrefab.h"

#include <FW_FileSystem.h>

void EntityPrefab::ComponentData::Parse(Slush::AssetParser::Handle aRootHandle)
{
	if (aRootHandle.IsReading())
	{
		Slush::AssetParser::Handle componentHandle = aRootHandle.ParseChildElement(myComponentDataName);
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
			Slush::AssetParser::Handle componentHandle = aRootHandle.ParseChildElement(myComponentDataName);
			if (componentHandle.IsValid())
				OnParse(componentHandle);
		}
	}
}

void EntityPrefab::ComponentData::BuildUI(FW_GrowingArray<MissingComponent>& someMissingComponentsOut)
{
	FW_String label = myComponentLabel;
	label += " Component";

	if (myEnabled)
	{
		if (ImGui::CollapsingHeader(label.GetBuffer(), &myEnabled))
		{
			ImGui::Indent();
			OnBuildUI();
			ImGui::Unindent();
		}
	}
	else
	{
		someMissingComponentsOut.Add({ label, &myEnabled });
	}
}

EntityPrefab::EntityPrefab(const char* aName)
	: DataAsset(aName)
	, myName(aName)
	, mySprite("Sprite", "sprite")
	, myAnimation("Animation", "animation")
	, myProjectileShooting("Projectille Shooting", "projectileshooting")
	, myPlayerController("Player Controller", "playercontroller")
	, myNPCController("NPC Controller", "npccontroller")
	, myHealth("Health", "health")
	, myPhysics("Physics", "physics")
	, myRemoveOnCollision("Remove On Collision", "removeoncollision")
	, myTargeting("Targeting", "targeting")
	, myWeaponComponent("Weapon", "weaponcomponent")
	, myExperience("Experience", "experience")
	, myPickup("Pickup", "pickup")
	, myStats("Stats", "stats")
	, myDamageDealer("Damage Dealer", "damagedealer")
	, myHealthBar("Health Bar", "healthbar")
{
	myComponentDatas.Fill(nullptr);

	/*myComponentDatas[GetComponentID<SpriteComponent>()] = new Sprite("Sprite", "sprite");
	myComponentDatas[GetComponentID<AnimationComponent>()] = new ComponentData("Animation", "animation");
	myComponentDatas[GetComponentID<ProjectileShootingComponent>()] = new ProjectileShooting("Projectile Shooting", "projectileshooting");
	myComponentDatas[GetComponentID<PlayerControllerComponent>()] = new ComponentData("Player Controller", "playercontroller");
	myComponentDatas[GetComponentID<NPCControllerComponent>()] = new ComponentData("NPC Controller", "npccontroller");
	myComponentDatas[GetComponentID<HealthComponent>()] = new Health("Health", "health");
	myComponentDatas[GetComponentID<PhysicsComponent>()] = new Physics("Physics", "physics");
	myComponentDatas[GetComponentID<RemoveOnCollisionComponent>()] = new ComponentData("Remove On Collision", "removeoncollision");
	myComponentDatas[GetComponentID<TargetingComponent>()] = new Targeting("Targeting", "targeting");
	myComponentDatas[GetComponentID<WeaponComponent>()] = new Weapon("Weapon", "weaponcomponent");
	myComponentDatas[GetComponentID<ExperienceComponent>()] = new ComponentData("Experience", "experience");
	myComponentDatas[GetComponentID<PickupComponent>()] = new ComponentData("Pickup", "pickup");
	myComponentDatas[GetComponentID<StatsComponent>()] = new Stats("Stats", "stats");
	myComponentDatas[GetComponentID<DamageDealerComponent>()] = new DamageDealer("Daamge Dealer", "damagedealer");
	myComponentDatas[GetComponentID<HealthBarComponent>()] = new ComponentData("Health Bar", "healthbar");*/


	myComponentDatas[GetComponentID<SpriteComponent>()] = &mySprite;
	myComponentDatas[GetComponentID<AnimationComponent>()] = &myAnimation;
	myComponentDatas[GetComponentID<ProjectileShootingComponent>()] = &myProjectileShooting;
	myComponentDatas[GetComponentID<PlayerControllerComponent>()] = &myPlayerController;
	myComponentDatas[GetComponentID<NPCControllerComponent>()] = &myNPCController;
	myComponentDatas[GetComponentID<HealthComponent>()] = &myHealth;
	myComponentDatas[GetComponentID<PhysicsComponent>()] = &myPhysics;
	myComponentDatas[GetComponentID<RemoveOnCollisionComponent>()] = &myRemoveOnCollision;
	myComponentDatas[GetComponentID<TargetingComponent>()] = &myTargeting;
	myComponentDatas[GetComponentID<WeaponComponent>()] = &myWeaponComponent;
	myComponentDatas[GetComponentID<ExperienceComponent>()] = &myExperience;
	myComponentDatas[GetComponentID<PickupComponent>()] = &myPickup;
	myComponentDatas[GetComponentID<StatsComponent>()] = &myStats;
	myComponentDatas[GetComponentID<DamageDealerComponent>()] = &myDamageDealer;
	myComponentDatas[GetComponentID<HealthBarComponent>()] = &myHealthBar;
}

EntityPrefab::~EntityPrefab()
{
	//myComponentDatas.DeleteAll();
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

	for (ComponentData* data : myComponentDatas)
	{
		if (data)
			data->Parse(aRootHandle);
	}
}

void EntityPrefab::BuildUI()
{
	ImGui::SeparatorText(myName.GetBuffer());

	const char* entityTypeNames[] = { "Environment", "Player", "NPC", "Player Projectile", "NPC Projectile", "Pickup" };
	ImGui::Combo("Entity Type", &myEntityType, entityTypeNames, IM_ARRAYSIZE(entityTypeNames));

	FW_GrowingArray<MissingComponent> missingComponents;

	for (ComponentData* data : myComponentDatas)
	{
		if (data)
			data->BuildUI(missingComponents);
	}

	BuildMissingComponentsUI(missingComponents);
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

void EntityPrefab::Sprite::OnBuildUI()
{
	ImGui::ColorEdit4("Color", myFloatColor);
	ImGui::InputFloat("Radius", &myRadius, 1.f, 10.f, "%.2f");
	ImGui::InputFloat2("Size", &mySize.x, "%.2f");

	myColor = FW_Float_To_ARGB(myFloatColor[3], myFloatColor[0], myFloatColor[1], myFloatColor[2]);
}

void EntityPrefab::ProjectileShooting::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("cooldown", myCooldown);
	aComponentHandle.ParseFloatField("projectilespeed", myProjectileSpeed);
	aComponentHandle.ParseFloatField("projectilespawnoffset", myProjectileSpawnOffset);
}

void EntityPrefab::ProjectileShooting::OnBuildUI()
{
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Cooldown", &myCooldown, 0.1f, 1.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Projectile Speed", &myProjectileSpeed, 1.f, 10.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Projectile Spawn Offset", &myProjectileSpawnOffset, 0.1f, 1.f, "%.2f");
}

void EntityPrefab::Health::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseIntField("maxhealth", myMaxHealth);
	aComponentHandle.ParseFloatField("graceperiodduration", myGracePeriodDuration);
}

void EntityPrefab::Health::OnBuildUI()
{
	ImGui::InputInt("Max Health", &myMaxHealth);
	ImGui::InputFloat("Grace Period Duration", &myGracePeriodDuration);
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

void EntityPrefab::Physics::OnBuildUI()
{
	ImGui::Checkbox("Is Static", &myStatic);
	ImGui::Checkbox("Is Sensor", &mySensor);
	ImGui::Checkbox("Match Sprite", &myMatchSprite);

	ImGui::InputFloat("Radius", &myRadius, 1.f, 10.f, "%.2f");
	ImGui::InputFloat2("Size", &mySize.x, "%.2f");
}

void EntityPrefab::Targeting::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	int targetTypeAsInt = myTargetType;
	aComponentHandle.ParseIntField("targettype", targetTypeAsInt);
	myTargetType = Entity::Type(targetTypeAsInt);
}

void EntityPrefab::Targeting::OnBuildUI()
{
	int type = myTargetType;
	const char* entityTypeNames[] = { "Environment", "Player", "NPC", "Player Projectile", "NPC Projectile", "Pickup" };
	ImGui::Combo("Target Type", &type, entityTypeNames, IM_ARRAYSIZE(entityTypeNames));
	myTargetType = Entity::Type(type);
}

void EntityPrefab::Stats::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseIntField("maxcooldownupgrades", myMaxCooldownUpgrades);
	aComponentHandle.ParseFloatField("cooldownperupgrade", myCooldownPerUpgrade);

	aComponentHandle.ParseIntField("maxdamageupgrades", myMaxDamageUpgrades);
	aComponentHandle.ParseFloatField("damageperupgrade", myDamagePerUpgrade);

	aComponentHandle.ParseIntField("maxadditionalprojectileupgrades", myMaxAdditionalProjectileUpgrades);
	aComponentHandle.ParseFloatField("additionalprojectilesperupgrade", myAdditionalProjectilesPerUpgrade);
}

void EntityPrefab::Stats::OnBuildUI()
{
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("Max Cooldown Upgrades", &myMaxCooldownUpgrades);

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Cooldown Per Upgrade", &myCooldownPerUpgrade, 0.05f, 1.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("Max Damage Upgrades", &myMaxDamageUpgrades);

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Damage Per Upgrade", &myDamagePerUpgrade, 0.05f, 1.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("Max Additional Projectiles Upgrades", &myMaxAdditionalProjectileUpgrades);

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Additional Projectiles Per Upgrade", &myAdditionalProjectilesPerUpgrade, 1.f, 1.f, "%.0f");
}

void EntityPrefab::DamageDealer::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseIntField("damage", myDamage);
}

void EntityPrefab::DamageDealer::OnBuildUI()
{
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("Damage", &myDamage);
}

void EntityPrefab::Weapon::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("basecooldown", myBaseCooldown);
	aComponentHandle.ParseFloatField("baseprojectilespeed", myBaseProjectileSpeed);
	aComponentHandle.ParseIntField("basedamage", myBaseDamage);
}

void EntityPrefab::Weapon::OnBuildUI()
{
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Base Cooldown", &myBaseCooldown, 0.05f, 0.1f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Base Projectile Speed", &myBaseProjectileSpeed, 1.f, 100.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("Base Damage", &myBaseDamage);
}
