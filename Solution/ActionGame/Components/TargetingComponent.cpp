#include "stdafx.h"

#include "EntitySystem/EntityManager.h"
#include "EntitySystem/EntityType.h"
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
	ImGui::Combo("Target Type", &type, GetEntityTypeNames(), EntityType::ENTITYTYPE_COUNT);
	myTargetType = EntityType(type);
}

//////////////////////////////////////////////////////////////////////////

TargetingComponent::TargetingComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
}

void TargetingComponent::Update()
{
	myTarget.Clear();

	const Data& targetData = myEntityPrefab.GetComponentData<TargetingComponent>();

	FW_GrowingArray<Slush::EntityHandle> targets;
	myEntity.myEntityManager.FindEntitiesOfType(targetData.myTargetType, targets);

	if (targetData.myTargetType == EntityType::PLAYER)
	{
		if (!targets.IsEmpty())
			myTarget = targets[0];
	}
	else
	{
		float bestDist = FLT_MAX;
		const float maxDist = FW_Square(600.f);
		for (const Slush::EntityHandle& handle : targets)
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