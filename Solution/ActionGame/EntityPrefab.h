#pragma once

#include "Entity.h"
#include <FW_String.h>
#include <FW_FileProcessor.h>
#include <FW_FileParser.h>
#include <Core\AssetParser.h>

class EntityPrefab
{
private:
	struct ComponentData
	{
		void Load(Slush::AssetParser::Handle aComponentHandle);
		void Save(const char* aComponentName, Slush::AssetParser::Handle aRootHandle);

		virtual void OnLoad(Slush::AssetParser::Handle aComponentHandle) { aComponentHandle; };
		virtual void OnSave(Slush::AssetParser::Handle aComponentHandle) { aComponentHandle; };

		bool myEnabled = false;
	};

public:
	EntityPrefab(const char* aName);

	void SaveToDisk();
	void Load(const char* aFilePath, bool aIsAbsolutePath);

	void BuildUI();
	bool BaseComponentUI(bool& aEnabledFlag, const char* aComponentLabel, const char* aAddComponentLabel);

	struct Sprite : public ComponentData
	{
		void OnSave(Slush::AssetParser::Handle aComponentHandle) override;
		void OnLoad(Slush::AssetParser::Handle aComponentHandle) override;

		float myRadius = 10.f;
		Vector2f mySize;
		int myColor = 0xFFFF3399;

		float myFloatColor[4] = { 1.f, 51.f / 255.f, 153.f / 255.f, 1.f };
	};
	
	struct ProjectileShooting : public ComponentData
	{
		void OnSave(Slush::AssetParser::Handle aComponentHandle) override;
		void OnLoad(Slush::AssetParser::Handle aComponentHandle) override;

		float myCooldown = 1.f;
	};

	struct Health : public ComponentData
	{
		void OnSave(Slush::AssetParser::Handle aComponentHandle) override;
		void OnLoad(Slush::AssetParser::Handle aComponentHandle) override;

		int myMaxHealth = 3;
	};

	struct Physics : public ComponentData
	{
		void OnSave(Slush::AssetParser::Handle aComponentHandle) override;
		void OnLoad(Slush::AssetParser::Handle aComponentHandle) override;

		bool myStatic = false;
		bool myMatchSprite = true;
		float myRadius = 10.f;
		Vector2f mySize;
	};

	struct Targeting : public ComponentData
	{
		void OnSave(Slush::AssetParser::Handle aComponentHandle) override;
		void OnLoad(Slush::AssetParser::Handle aComponentHandle) override;

		Entity::Type myTargetType;
	};

	FW_String myName;
	int myEntityType;

	Sprite mySprite;
	ComponentData myAnimation;
	ProjectileShooting myProjectileShooting;
	ComponentData myPlayerController;
	ComponentData myNPCController;
	Health myHealth;
	Physics myPhysics;
	ComponentData myRemoveOnCollision;
	Targeting myTargeting;
	ComponentData myWeaponComponent;
	ComponentData myExperience;
	ComponentData myPickup;
};
