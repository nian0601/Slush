#include "stdafx.h"

#include "EntityManager.h"
#include "EntityPrefab.h"
#include "PhysicsComponent.h"
#include "TargetingComponent.h"

#include <Physics\PhysicsWorld.h>
#include <float.h>

void TargetingComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	int targetTypeAsInt = myTargetType;
	aComponentHandle.ParseIntField("targettype", targetTypeAsInt);
	myTargetType = EntityType(targetTypeAsInt);
}

void TargetingComponent::Data::OnBuildUI()
{
	int type = myTargetType;
	const char* entityTypeNames[] = { "Environment", "Player", "NPC", "Player Projectile", "NPC Projectile", "Pickup" };
	ImGui::Combo("Target Type", &type, entityTypeNames, IM_ARRAYSIZE(entityTypeNames));
	myTargetType = EntityType(type);
}

//////////////////////////////////////////////////////////////////////////

TargetingComponent::TargetingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
}

void TargetingComponent::Update()
{
	myTarget.Clear();

	const Data& targetData = myEntityPrefab.GetTargetingData();

	FW_GrowingArray<EntityHandle> targets;
	myEntity.myEntityManager.FindEntitiesOfType(static_cast<EntityType>(targetData.myTargetType), targets);

	if (targetData.myTargetType == EntityType::PLAYER)
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