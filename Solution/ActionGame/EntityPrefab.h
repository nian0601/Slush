#pragma once

#include "Entity.h"
#include <FW_String.h>

class EntityPrefab
{
public:
	EntityPrefab(const char* aName);

	FW_String myName;
	Entity::Type myEntityType;

	struct Sprite
	{
		bool myEnabled = false;
		float myRadius = 0.f;
		Vector2f mySize;
		int myColor = 0;
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
		float myRadius = 0.f;
		Vector2f mySize;
	} myPhysics;

	struct RemoveOnCollision
	{
		bool myEnabled = false;
	} myRemoveOnCollision;
};
