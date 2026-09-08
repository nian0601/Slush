#include "stdafx.h"

#include "TowerCombatComponent.h"

#include <float.h>
#include <EntitySystem/Entity.h>
#include <EntitySystem/EntityManager.h>
#include <imgui/ImGuiWidgets.h>

#include "HealthComponent.h"
#include "MovementComponent.h"
#include "TargetableComponent.h"

void TowerCombatComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseFloatField("range", myRange);
	aComponentHandle.ParseFloatField("cooldown", myCooldown);
	aComponentHandle.ParseIntField("damage", myDamage);
}

void TowerCombatComponent::Data::OnBuildUI()
{
	Slush::ImGuiWidgets::InputFloat("Range", &myRange);
	Slush::ImGuiWidgets::InputFloat("Cooldown", &myCooldown);
	Slush::ImGuiWidgets::InputInt("Damage", &myDamage);
}

TowerCombatComponent::TowerCombatComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	const Data& data = anEntityPrefab.GetComponentData<TowerCombatComponent>();
	myRange = data.myRange;
	myCooldown = data.myCooldown;
	myDamage = data.myDamage;

	SLUSH_DEBUG("Tower '%s' created at (%.1f, %.1f) with damage %d", myEntityPrefab.GetAssetName().GetBuffer(), myEntity.myPosition.x, myEntity.myPosition.y, myDamage);
}

void TowerCombatComponent::Update()
{
	Slush::Entity* target = myTarget.Get();
	if (!target || target->myIsMarkedForRemoval || !IsTargetInRange(*target))
	{
		myTarget.Clear();
		AcquireTarget();
	}

	if (!myTarget.IsValid())
		return;

	if (!myAttackTimer.IsStarted() || myAttackTimer.HasExpired())
	{
		AttackTarget();
		myAttackTimer.Start(myCooldown);
	}
}

bool TowerCombatComponent::IsTargetInRange(const Slush::Entity& anEntity) const
{
	return Length2(anEntity.myPosition - myEntity.myPosition) <= FW_Square(myRange);
}

void TowerCombatComponent::AcquireTarget()
{
	FW_GrowingArray<Slush::EntityHandle> entities;
	myEntity.myEntityManager.GetAllEntities(entities);

	float bestRemainingPathDistance = FLT_MAX;
	for (const Slush::EntityHandle& handle : entities)
	{
		Slush::Entity* entity = handle.Get();
		if (!entity || entity->myIsMarkedForRemoval || !entity->GetComponent<TargetableComponent>() || !IsTargetInRange(*entity))
			continue;

		MovementComponent* movement = entity->GetComponent<MovementComponent>();
		FW_ASSERT(movement, "Tower target is missing a MovementComponent");

		const float remainingPathDistance = movement->GetRemainingPathDistance();
		if (remainingPathDistance < bestRemainingPathDistance)
		{
			myTarget = handle;
			bestRemainingPathDistance = remainingPathDistance;
		}
	}
}

void TowerCombatComponent::AttackTarget()
{
	Slush::Entity* target = myTarget.Get();
	if (!target)
		return;

	HealthComponent* health = target->GetComponent<HealthComponent>();
	FW_ASSERT(health, "Tower target is missing a HealthComponent");

	health->DealDamage(myDamage);
	if (health->IsDead())
	{
		SLUSH_DEBUG("Tower '%s' at (%.1f, %.1f) killed a target at (%.1f, %.1f) with damage %d", myEntityPrefab.GetAssetName().GetBuffer(), myEntity.myPosition.x, myEntity.myPosition.y, target->myPosition.x, target->myPosition.y, myDamage);
	}
}
