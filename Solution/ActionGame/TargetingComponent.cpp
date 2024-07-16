#include "stdafx.h"

#include "EntityManager.h"
#include "EntityPrefab.h"
#include "PhysicsComponent.h"
#include "TargetingComponent.h"

#include <Physics\PhysicsWorld.h>
#include <float.h>

TargetingComponent::TargetingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
}

void TargetingComponent::Update()
{
	myTarget.Clear();

	FW_GrowingArray<EntityHandle> targets;
	myEntity.myEntityManager.FindEntitiesOfType(myEntityPrefab.myTargeting.myTargetType, targets);

	if (myEntityPrefab.myTargeting.myTargetType == Entity::PLAYER)
	{
		if (!targets.IsEmpty())
			myTarget = targets[0];
	}
	else
	{
		float bestDist = FLT_MAX;
		const float maxDist = FW_Square(600.f);
		for (const EntityHandle& handle : targets)
		{
			if (handle.IsValid())
			{
				const float dist = Length2(myEntity.myPosition - handle.Get()->myPosition);
				if (dist < maxDist && dist < bestDist)
				{
					myTarget = handle;
					bestDist = dist;
				}
			}
		}
	}
}