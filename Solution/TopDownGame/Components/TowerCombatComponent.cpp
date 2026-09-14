#include "stdafx.h"

#include "TowerCombatComponent.h"

#include <EntitySystem/Entity.h>
#include <EntitySystem/EntityManager.h>
#include <imgui/ImGuiWidgets.h>

#include "MovementComponent.h"
#include "ProjectileComponent.h"
#include "TargetableComponent.h"

void TowerCombatComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseFloatField("range", myRange);
	aComponentHandle.ParseFloatField("cooldown", myCooldown);
	myProjectilePrefab.Parse(aComponentHandle, "projectiletype");
}

void TowerCombatComponent::Data::ResolveDependencies()
{
	myProjectilePrefab.ResolveDependency();
}

void TowerCombatComponent::Data::OnBuildUI()
{
	Slush::ImGuiWidgets::InputFloat("Range", &myRange);
	Slush::ImGuiWidgets::InputFloat("Cooldown", &myCooldown);
	ImGui::Text("ProjectileType: %s", myProjectilePrefab.GetName().GetBuffer());
}

TowerCombatComponent::TowerCombatComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	const Data& data = anEntityPrefab.GetComponentData<TowerCombatComponent>();
	myRange = data.myRange;
	myCooldown = data.myCooldown;

	SLUSH_DEBUG("Tower '%s' created at (%.1f, %.1f)", myEntityPrefab.GetAssetName().GetBuffer(), myEntity.myPosition.x, myEntity.myPosition.y);
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

	float bestRemainingPathDistance = 0.f;
	bool hasTarget = false;
	for (const Slush::EntityHandle& handle : entities)
	{
		Slush::Entity* entity = handle.Get();
		if (!entity || entity->myIsMarkedForRemoval || !entity->GetComponent<TargetableComponent>() || !IsTargetInRange(*entity))
			continue;

		MovementComponent* movement = entity->GetComponent<MovementComponent>();
		const float remainingPathDistance = movement ? movement->GetRemainingPathDistance() : 0.f;
		if (!hasTarget || remainingPathDistance < bestRemainingPathDistance)
		{
			myTarget = handle;
			bestRemainingPathDistance = remainingPathDistance;
			hasTarget = true;
		}
	}
}

void TowerCombatComponent::AttackTarget()
{
	Slush::Entity* target = myTarget.Get();
	if (!target)
		return;

	const Data& data = myEntityPrefab.GetComponentData<TowerCombatComponent>();
	if (Slush::EntityPrefab* projectilePrefab = data.myProjectilePrefab.Get())
	{
		Slush::Entity* projectileEntity = myEntity.myEntityManager.CreateEntity(myEntity.myPosition, *projectilePrefab);
		ProjectileComponent* projectile = projectileEntity->GetComponent<ProjectileComponent>();
		FW_ASSERT(projectile, "Tower projectile is missing a ProjectileComponent");
		projectile->SetTarget(myTarget);
	}
}
