#include "stdafx.h"

#include "RemoveOnCollisionComponent.h"
#include "Entity.h"

void RemoveOnCollisionComponent::OnCollision(Entity& aOtherEntity)
{
	if (aOtherEntity.myType != Entity::ENVIRONMENT)
		myEntity.myIsMarkedForRemoval = true;
}

