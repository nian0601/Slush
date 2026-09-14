#include "stdafx.h"

#include "ProjectileComponent.h"

#include <Core/Time.h>
#include <EntitySystem/Entity.h>
#include <imgui/ImGuiWidgets.h>

#include "HealthComponent.h"

void ProjectileComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseFloatField("speed", mySpeed);
	aComponentHandle.ParseIntField("damage", myDamage);
}

void ProjectileComponent::Data::OnBuildUI()
{
	Slush::ImGuiWidgets::InputFloat("Speed", &mySpeed);
	Slush::ImGuiWidgets::InputInt("Damage", &myDamage);
}

ProjectileComponent::ProjectileComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	const Data& data = anEntityPrefab.GetComponentData<ProjectileComponent>();
	mySpeed = data.mySpeed;
	myDamage = data.myDamage;
}

void ProjectileComponent::Update()
{
	Slush::Entity* target = myTarget.Get();
	if (!target || target->myIsMarkedForRemoval)
	{
		myEntity.myIsMarkedForRemoval = true;
		return;
	}

	const Vector2f toTarget = target->myPosition - myEntity.myPosition;
	const float distanceToTarget = Length(toTarget);
	const float stepDistance = mySpeed * Slush::Time::GetDelta();
	if (distanceToTarget <= stepDistance)
	{
		HealthComponent* health = target->GetComponent<HealthComponent>();
		FW_ASSERT(health, "Projectile target is missing a HealthComponent");

		health->DealDamage(myDamage);
		if (health->IsDead())
		{
			SLUSH_DEBUG("Projectile '%s' at (%.1f, %.1f) killed a target at (%.1f, %.1f) with damage %d", myEntityPrefab.GetAssetName().GetBuffer(), myEntity.myPosition.x, myEntity.myPosition.y, target->myPosition.x, target->myPosition.y, myDamage);
		}

		myEntity.myIsMarkedForRemoval = true;
		return;
	}

	myEntity.myPosition += GetNormalized(toTarget) * stepDistance;
}

void ProjectileComponent::SetTarget(const Slush::EntityHandle& aTarget)
{
	myTarget = aTarget;
}
