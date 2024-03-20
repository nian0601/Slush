#include "TargetingComponent.h"
#include "Entity.h"
#include "EntityPrefab.h"
#include "EntityManager.h"
#include "PhysicsComponent.h"
#include <Physics\PhysicsWorld.h>
#include <float.h>

TargetingComponent::TargetingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
}

void TargetingComponent::Update()
{
	if (!myTarget.IsValid())
	{
		FW_GrowingArray<EntityHandle> targets;
		myEntity.myEntityManager.FindEntitiesOfType(myEntityPrefab.myTargeting.myTargetType, targets);

		float bestDist = FLT_MAX;
		for (const EntityHandle& handle : targets)
		{
			if (handle.IsValid())
			{
				float dist = Length2(myEntity.myPosition - handle.Get()->myPosition);
				if (dist < bestDist)
				{
					myTarget = handle;
					dist = bestDist;
				}
			}
		}
	}
}