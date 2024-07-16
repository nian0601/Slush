#include "stdafx.h"

#include "EntityPrefab.h"
#include "PhysicsComponent.h"

#include <Physics\PhysicsWorld.h>

namespace
{
	enum CollisionFlags
	{
		COL_ENVIRONMENT = 1 << 1,
		COL_PLAYER = 1 << 2,
		COL_NPC = 1 << 3,
		COL_PLAYER_PROJECTILE = 1 << 4,
		COL_NPC_PROJECTILE = 1 << 5,
		COL_PICKUP = 1 << 6
	};

	unsigned int GetCollisionFlag(Entity::Type anEntityType)
	{
		switch (anEntityType)
		{
		case Entity::ENVIRONMENT:
			return COL_ENVIRONMENT;
		case Entity::PLAYER:
			return COL_PLAYER;
		case Entity::NPC:
			return COL_NPC;
		case Entity::PLAYER_PROJECTILE:
			return COL_PLAYER_PROJECTILE;
		case Entity::NPC_PROJECTILE:
			return COL_NPC_PROJECTILE;
		case Entity::PICKUP:
			return COL_PICKUP;
		default:
			FW_ASSERT_ALWAYS("Unhandled EntityType");
			return 0;
		}
	}

	unsigned int GetCollidesWithFlag(Entity::Type anEntityType)
	{
		switch (anEntityType)
		{
		case Entity::ENVIRONMENT:
			return COL_PLAYER | COL_NPC | COL_PLAYER_PROJECTILE | COL_NPC_PROJECTILE;
		case Entity::PLAYER:
			return COL_ENVIRONMENT | COL_NPC | COL_NPC_PROJECTILE | COL_PICKUP;
		case Entity::NPC:
			return COL_ENVIRONMENT | COL_PLAYER | COL_PLAYER_PROJECTILE;
		case Entity::PLAYER_PROJECTILE:
			return COL_ENVIRONMENT | COL_NPC;
		case Entity::NPC_PROJECTILE:
			return COL_ENVIRONMENT | COL_PLAYER;
		case Entity::PICKUP:
			return COL_PLAYER;
		default:
			FW_ASSERT_ALWAYS("Unhandled EntityType");
			return 0;
		}
	}
}

PhysicsComponent::PhysicsComponent(Entity& aEntity, const EntityPrefab& anEntityPrefab, Slush::PhysicsWorld& aPhysicsWorld)
	: Component(aEntity, anEntityPrefab)
	, myPhysicsWorld(aPhysicsWorld)
{
	Slush::PhysicsShape* shape = nullptr;
	if (anEntityPrefab.myPhysics.myMatchSprite)
	{
		if (anEntityPrefab.mySprite.myEnabled)
		{
			if (anEntityPrefab.mySprite.mySize.x > 0.f)
				shape = new Slush::AABBShape(anEntityPrefab.mySprite.mySize);
			else
				shape = new Slush::CircleShape(anEntityPrefab.mySprite.myRadius);
		}
		else
		{
			SLUSH_ERROR("Failed to match Physics to Sprite since EntityPrefab doesnt have Sprite enabled, creating a Unit-circle as default");
			shape = new Slush::CircleShape(1.f);
		}
	}
	else
	{
		if (anEntityPrefab.myPhysics.mySize.x > 0.f)
			shape = new Slush::AABBShape(anEntityPrefab.myPhysics.mySize);
		else
			shape = new Slush::CircleShape(anEntityPrefab.myPhysics.myRadius);
	}

	if (shape)
	{
		myObject = new Slush::PhysicsObject(shape);
		myObject->SetPosition(myEntity.myPosition);
		myObject->myUserData.Set<PhysicsComponent* const>(this);
		myObject->myCollisionMask = GetCollisionFlag(aEntity.myType);
		myObject->myCollidesWithMask = GetCollidesWithFlag(aEntity.myType);
		myObject->myReportCollisionsWith = myObject->myCollidesWithMask;

		aPhysicsWorld.AddObject(myObject);

		if (anEntityPrefab.myPhysics.myStatic)
			myObject->MakeStatic();

		if (anEntityPrefab.myPhysics.mySensor)
			myObject->MakeSensor();
	}
	else
	{
		SLUSH_ERROR("Failed to create PhysicsShape, wont create PhysicsComponent for the entity");
	}
}

PhysicsComponent::~PhysicsComponent()
{
	myPhysicsWorld.RemoveObject(myObject);
	FW_SAFE_DELETE(myObject);
}

void PhysicsComponent::Update()
{	
	myEntity.myPosition = myObject->myPosition;
}