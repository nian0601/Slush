#include "PhysicsComponent.h"

#include "Entity.h"
#include <Physics\PhysicsWorld.h>
#include <FW_Includes.h>
#include "EntityPrefab.h"
#include <Core\Log.h>

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
		aPhysicsWorld.AddObject(myObject);

		if (anEntityPrefab.myPhysics.myStatic)
			myObject->MakeStatic();
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