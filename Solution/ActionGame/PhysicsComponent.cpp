#include "PhysicsComponent.h"

#include "Entity.h"
#include <Physics\PhysicsWorld.h>

void PhysicsComponent::Update()
{
	
	myEntity.myPosition = myObject->myPosition;
}