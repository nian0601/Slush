#include "stdafx.h"

#include "EntityManager.h"
#include "EntityPrefab.h"
#include "PhysicsComponent.h"
#include "ProjectileShootingComponent.h"

#include <Physics\PhysicsWorld.h>

ProjectileShootingComponent::Data::Data()
	: BaseData("Projectile Shooting", "projectileshooting")
{}

void ProjectileShootingComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("cooldown", myCooldown);
	aComponentHandle.ParseFloatField("projectilespeed", myProjectileSpeed);
	aComponentHandle.ParseFloatField("projectilespawnoffset", myProjectileSpawnOffset);
}

void ProjectileShootingComponent::Data::OnBuildUI()
{
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Cooldown", &myCooldown, 0.1f, 1.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Projectile Speed", &myProjectileSpeed, 1.f, 10.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Projectile Spawn Offset", &myProjectileSpawnOffset, 0.1f, 1.f, "%.2f");
}

//////////////////////////////////////////////////////////////////////////

ProjectileShootingComponent::ProjectileShootingComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
}

void ProjectileShootingComponent::TryShoot(const Vector2f& aDirection)
{
	if (Slush::Time::GetCurrentExactTime() < myShootingReadyTimestamp)
		return;

	TriggerCooldown();

	const char* prefab = "PlayerProjectile";
	if (myEntity.myType == EntityType::NPC)
		prefab = "NPCProjectile";

	const Data& shootingData = myEntityPrefab.GetProjectileShootingData();

	Vector2f projPosition = myEntity.myPosition + aDirection * shootingData.myProjectileSpawnOffset;
	Entity* projectile = myEntity.myEntityManager.CreateEntity(projPosition, prefab);
	projectile->GetComponent<PhysicsComponent>()->myObject->myVelocity = aDirection * shootingData.myProjectileSpeed;
}

void ProjectileShootingComponent::TriggerCooldown()
{
	myShootingReadyTimestamp = Slush::Time::GetCurrentExactTime() + Slush::Time::ConvertGameTimeToTimeUnit(myEntityPrefab.GetProjectileShootingData().myCooldown);
}
