#include "stdafx.h"

#include "EntitySystem/EntityManager.h"
#include "EntitySystem/Components/PhysicsComponent.h"
#include "EntitySystem/Components/SpriteComponent.h"
#include "ProjectileShootingComponent.h"
#include "ActionGameGlobals.h"

#include <Physics\PhysicsWorld.h>
#include <Graphics\BaseSprite.h>

void ProjectileShootingComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseFloatField("cooldown", myCooldown);
	aComponentHandle.ParseFloatField("projectilespeed", myProjectileSpeed);
	aComponentHandle.ParseFloatField("projectilespawnoffset", myProjectileSpawnOffset);
	myProjectileEntityPrefab.Parse(aComponentHandle, "projectiletype");
}

void ProjectileShootingComponent::Data::ResolveDependencies()
{
	myProjectileEntityPrefab.ResolveDependency();
}

void ProjectileShootingComponent::Data::OnBuildUI()
{
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Cooldown", &myCooldown, 0.1f, 1.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Projectile Speed", &myProjectileSpeed, 1.f, 10.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Projectile Spawn Offset", &myProjectileSpawnOffset, 0.1f, 1.f, "%.2f");

	ImGui::Text("ProjectileType: %s", myProjectileEntityPrefab.GetName().GetBuffer());

	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<Slush::EntityPrefab>()))
			myProjectileEntityPrefab.Set(static_cast<Slush::EntityPrefab*>(asset));

		ImGui::EndDragDropTarget();
	}
}

//////////////////////////////////////////////////////////////////////////

ProjectileShootingComponent::ProjectileShootingComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
}

bool ProjectileShootingComponent::TryShoot(const Vector2f& aDirection)
{
	if (Slush::Time::GetCurrentExactTime() < myShootingReadyTimestamp)
		return false;

	TriggerCooldown();

	const ProjectileShootingComponent::Data& shootingData = myEntityPrefab.GetComponentData<ProjectileShootingComponent>();

	Slush::EntityPrefab* prefab = shootingData.myProjectileEntityPrefab.Get();
	if (!prefab)
		return true;

	Vector2f projPosition = myEntity.myPosition + aDirection * shootingData.myProjectileSpawnOffset;
	Slush::Entity* projectile = myEntity.myEntityManager.CreateEntity(projPosition, *prefab);
	projectile->GetComponent<Slush::PhysicsComponent>()->myObject->myVelocity = aDirection * shootingData.myProjectileSpeed;
	projectile->GetComponent<Slush::SpriteComponent>()->GetSprite().SetRotation(FW_SignedAngle(aDirection));

	return true;
}

void ProjectileShootingComponent::TriggerCooldown()
{
	myShootingReadyTimestamp = Slush::Time::GetCurrentExactTime() + Slush::Time::ConvertGameTimeToTimeUnit(myEntityPrefab.GetComponentData<ProjectileShootingComponent>().myCooldown);
}
