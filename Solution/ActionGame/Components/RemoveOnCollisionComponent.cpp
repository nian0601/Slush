#include "stdafx.h"

#include "RemoveOnCollisionComponent.h"

void RemoveOnCollisionComponent::OnCollision(Entity& aOtherEntity, const Vector2f& /*aContactPosition*/)
{
	if (aOtherEntity.myType != EntityType::ENVIRONMENT)
		myEntity.myIsMarkedForRemoval = true;
}

