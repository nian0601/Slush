#include "PhysicsComponent.h"

#include "Entity.h"
#include <Physics\PhysicsWorld.h>
#include <FW_Includes.h>

PhysicsComponent::PhysicsComponent(Entity& aEntity, Slush::PhysicsWorld& aPhysicsWorld)
	: Component(aEntity)
	, myPhysicsWorld(aPhysicsWorld)
{
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