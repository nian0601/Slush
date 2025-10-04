#include "stdafx.h"

#include "RemoveOnCollisionComponent.h"

void RemoveOnCollisionComponent::OnCollision(Entity& aOtherEntity)
{
	if (aOtherEntity.myType != EntityType::ENVIRONMENT)
		myEntity.myIsMarkedForRemoval = true;
}

