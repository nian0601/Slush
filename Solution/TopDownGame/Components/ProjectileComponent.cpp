#include "stdafx.h"

#include "ProjectileComponent.h"

#include <Core/Time.h>
#include <EntitySystem/Entity.h>
#include <EntitySystem/EntityManager.h>
#include <imgui/ImGuiWidgets.h>

#include "HealthComponent.h"
#include "TargetableComponent.h"

void ProjectileComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseFloatField("speed", mySpeed);
	aComponentHandle.ParseIntField("damage", myDamage);
	aComponentHandle.ParseFloatField("splashradius", mySplashRadius);
}

void ProjectileComponent::Data::OnBuildUI()
{
	Slush::ImGuiWidgets::InputFloat("Speed", &mySpeed);
	Slush::ImGuiWidgets::InputInt("Damage", &myDamage);
	Slush::ImGuiWidgets::InputFloat("Splash Radius", &mySplashRadius);
}

ProjectileComponent::ProjectileComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	const Data& data = anEntityPrefab.GetComponentData<ProjectileComponent>();
	mySpeed = data.mySpeed;
	myDamage = data.myDamage;
	mySplashRadius = data.mySplashRadius;
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
		FW_GrowingArray<Slush::EntityHandle> entities;
		myEntity.myEntityManager.GetAllEntities(entities);

		for (const Slush::EntityHandle& handle : entities)
		{
			Slush::Entity* entity = handle.Get();
			if (!entity || entity->myIsMarkedForRemoval || !entity->GetComponent<TargetableComponent>() || Length2(entity->myPosition - target->myPosition) > FW_Square(mySplashRadius))
				continue;

			HealthComponent* health = entity->GetComponent<HealthComponent>();
			if (!health || health->IsDead())
				continue;

			health->DealDamage(myDamage);
			if (health->IsDead())
			{
				SLUSH_DEBUG("Projectile '%s' at (%.1f, %.1f) killed a target at (%.1f, %.1f) with damage %d", myEntityPrefab.GetAssetName().GetBuffer(), myEntity.myPosition.x, myEntity.myPosition.y, entity->myPosition.x, entity->myPosition.y, myDamage);
			}
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
