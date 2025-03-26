#include "stdafx.h"

#include "EntityPrefab.h"
#include "PhysicsComponent.h"

#include <Physics\PhysicsWorld.h>
#include "ActionGameGlobals.h"

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

	unsigned int GetCollisionFlag(EntityType anEntityType)
	{
		switch (anEntityType)
		{
		case EntityType::ENVIRONMENT:
			return COL_ENVIRONMENT;
		case EntityType::PLAYER:
			return COL_PLAYER;
		case EntityType::NPC:
			return COL_NPC;
		case EntityType::PLAYER_PROJECTILE:
			return COL_PLAYER_PROJECTILE;
		case EntityType::NPC_PROJECTILE:
			return COL_NPC_PROJECTILE;
		case EntityType::PICKUP:
			return COL_PICKUP;
		default:
			FW_ASSERT_ALWAYS("Unhandled EntityType");
			return 0;
		}
	}

	unsigned int GetCollidesWithFlag(EntityType anEntityType)
	{
		switch (anEntityType)
		{
		case EntityType::ENVIRONMENT:
			return COL_PLAYER | COL_NPC | COL_PLAYER_PROJECTILE | COL_NPC_PROJECTILE;
		case EntityType::PLAYER:
			return COL_ENVIRONMENT | COL_NPC | COL_NPC_PROJECTILE | COL_PICKUP;
		case EntityType::NPC:
			return COL_ENVIRONMENT | COL_PLAYER | COL_PLAYER_PROJECTILE;
		case EntityType::PLAYER_PROJECTILE:
			return COL_ENVIRONMENT | COL_NPC;
		case EntityType::NPC_PROJECTILE:
			return COL_ENVIRONMENT | COL_PLAYER;
		case EntityType::PICKUP:
			return COL_PLAYER;
		default:
			FW_ASSERT_ALWAYS("Unhandled EntityType");
			return 0;
		}
	}
}

PhysicsComponent::PhysicsComponent(Entity& aEntity, const EntityPrefab& anEntityPrefab)
	: Component(aEntity, anEntityPrefab)
	, myPhysicsWorld(ActionGameGlobals::GetInstance().GetPhysicsWorld())
{
	const EntityPrefab::Physics& physData = anEntityPrefab.GetPhysicsData();

	Slush::PhysicsShape* shape = nullptr;
	if (physData.myMatchSprite)
	{
		if (anEntityPrefab.Has<SpriteComponent>())
		{
			const EntityPrefab::Sprite& spriteData = anEntityPrefab.GetSpriteData();
			if (spriteData.mySize.x > 0.f)
				shape = new Slush::AABBShape(spriteData.mySize);
			else
				shape = new Slush::CircleShape(spriteData.myRadius);
		}
		else
		{
			SLUSH_ERROR("Failed to match Physics to Sprite since EntityPrefab doesnt have Sprite enabled, creating a Unit-circle as default");
			shape = new Slush::CircleShape(1.f);
		}
	}
	else
	{
		if (physData.mySize.x > 0.f)
			shape = new Slush::AABBShape(physData.mySize);
		else
			shape = new Slush::CircleShape(physData.myRadius);
	}

	if (shape)
	{
		myObject = new Slush::PhysicsObject(shape);
		myObject->SetPosition(myEntity.myPosition);
		myObject->myUserData.Set<PhysicsComponent* const>(this);
		myObject->myCollisionMask = GetCollisionFlag(aEntity.myType);
		myObject->myCollidesWithMask = GetCollidesWithFlag(aEntity.myType);
		myObject->myReportCollisionsWith = myObject->myCollidesWithMask;

		myPhysicsWorld.AddObject(myObject);

		if (physData.myStatic)
			myObject->MakeStatic();

		if (physData.mySensor)
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