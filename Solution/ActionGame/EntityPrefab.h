#pragma once

#include "Entity.h"
#include <FW_String.h>

class EntityPrefab
{
public:
	EntityPrefab(const char* aName);

	void BuildUI();
	bool BaseComponentUI(bool& aEnabledFlag, const char* aComponentLabel, const char* aAddComponentLabel);

	FW_String myName;
	Entity::Type myEntityType;

	struct Sprite
	{
		bool myEnabled = false;
		float myRadius = 10.f;
		Vector2f mySize;
		int myColor = 0xFFFF3399;

		float myFloatColor[4] = { 1.f, 51.f / 255.f, 153.f / 255.f, 1.f };
	} mySprite;

	struct Animation
	{
		bool myEnabled = false;
	} myAnimation;
	
	struct ProjectileShooting
	{
		bool myEnabled = false;
		float myCooldown = 1.f;
	} myProjectileShooting;
	
	struct PlayerController
	{
		bool myEnabled = false;
	} myPlayerController;

	struct NPCController
	{
		bool myEnabled = false;
	} myNPCController;

	struct Health
	{
		bool myEnabled = false;
		int myMaxHealth = 3;
	} myHealth;

	struct Physics
	{
		bool myEnabled = false;
		bool myStatic = false;
		bool myMatchSprite = true;
		float myRadius = 10.f;
		Vector2f mySize;
	} myPhysics;

	struct RemoveOnCollision
	{
		bool myEnabled = false;
	} myRemoveOnCollision;
};
