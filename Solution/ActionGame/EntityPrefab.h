#pragma once

#include "Entity.h"
#include <FW_String.h>
#include <FW_FileProcessor.h>
#include <FW_FileParser.h>

class EntityPrefab
{
public:
	EntityPrefab(const char* aName);

	void SaveToDisk();
	void Load(const char* aFilePath, bool aIsAbsolutePath);
	void LoadEmptyComponent(FW_FileParser& aParser);

	void BuildUI();
	bool BaseComponentUI(bool& aEnabledFlag, const char* aComponentLabel, const char* aAddComponentLabel);

	FW_String myName;
	int myEntityType;

	struct Sprite
	{
		void SaveToDisk(FW_FileProcessor& aProcessor);
		void LoadFromDisk(FW_FileParser& aParser);

		bool myEnabled = false;
		float myRadius = 10.f;
		Vector2f mySize;
		int myColor = 0xFFFF3399;

		float myFloatColor[4] = { 1.f, 51.f / 255.f, 153.f / 255.f, 1.f };
	} mySprite;

	struct Animation
	{
		void SaveToDisk(FW_FileProcessor& aProcessor);

		bool myEnabled = false;
	} myAnimation;
	
	struct ProjectileShooting
	{
		void SaveToDisk(FW_FileProcessor& aProcessor);
		void LoadFromDisk(FW_FileParser& aParser);

		bool myEnabled = false;
		float myCooldown = 1.f;
	} myProjectileShooting;
	
	struct PlayerController
	{
		void SaveToDisk(FW_FileProcessor& aProcessor);

		bool myEnabled = false;
	} myPlayerController;

	struct NPCController
	{
		void SaveToDisk(FW_FileProcessor& aProcessor);

		bool myEnabled = false;
	} myNPCController;

	struct Health
	{
		void SaveToDisk(FW_FileProcessor& aProcessor);
		void LoadFromDisk(FW_FileParser& aParser);

		bool myEnabled = false;
		int myMaxHealth = 3;
	} myHealth;

	struct Physics
	{
		void SaveToDisk(FW_FileProcessor& aProcessor);
		void LoadFromDisk(FW_FileParser& aParser);

		bool myEnabled = false;
		bool myStatic = false;
		bool myMatchSprite = true;
		float myRadius = 10.f;
		Vector2f mySize;
	} myPhysics;

	struct RemoveOnCollision
	{
		void SaveToDisk(FW_FileProcessor& aProcessor);

		bool myEnabled = false;
	} myRemoveOnCollision;

	struct Targeting
	{
		void SaveToDisk(FW_FileProcessor& aProcessor);
		void LoadFromDisk(FW_FileParser& aParser);

		bool myEnabled = false;
		Entity::Type myTargetType;
	} myTargeting;
};
