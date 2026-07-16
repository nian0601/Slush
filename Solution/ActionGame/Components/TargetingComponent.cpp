#include "stdafx.h"

#include "EntitySystem/EntityManager.h"
#include "TargetingComponent.h"

#include <Physics\PhysicsWorld.h>
#include <float.h>

void TargetingComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseIntField("targetflag", myTargetFlag);
}

void TargetingComponent::Data::OnBuildUI()
{
	ImGui::Combo("Target Flag", &myTargetFlag, CollisionUtils::GetNames(), CollisionUtils::COLLISIONFLAG_COUNT);
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

	FW_GrowingArray<Slush::EntityHandle> allEntities;
	myEntity.myEntityManager.GetAllEntities(allEntities);

	FW_GrowingArray<Slush::EntityHandle> targets;
	for (const Slush::EntityHandle& handle : allEntities)
	{
		if (!handle.IsValid())
			continue;

		Slush::PhysicsComponent* physics = handle.Get()->GetComponent<Slush::PhysicsComponent>();
		if (physics && physics->GetCollisionFlag() == targetData.myTargetFlag)
			targets.Add(handle);
	}

	if (targetData.myTargetFlag == CollisionUtils::COL_PLAYER)
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
			const float dist = Length2(myEntity.myPosition - handle.Get()->myPosition);
			if (dist < maxDist && dist < bestDist)
			{
				myTarget = handle;
				bestDist = dist;
			}
		}
	}
}