#include "stdafx.h"

#include "EntityManager.h"
#include "EntityPrefab.h"
#include "PhysicsComponent.h"
#include "ProjectileShootingComponent.h"
#include "SpriteComponent.h"
#include "ActionGameGlobals.h"

#include <Physics\PhysicsWorld.h>
#include <Graphics\BaseSprite.h>

void ProjectileShootingComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("cooldown", myCooldown);
	aComponentHandle.ParseFloatField("projectilespeed", myProjectileSpeed);
	aComponentHandle.ParseFloatField("projectilespawnoffset", myProjectileSpawnOffset);
	aComponentHandle.ParseStringField("projectiletype", myProjectileEntityPrefab);
}

void ProjectileShootingComponent::Data::OnBuildUI()
{
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Cooldown", &myCooldown, 0.1f, 1.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Projectile Speed", &myProjectileSpeed, 1.f, 10.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Projectile Spawn Offset", &myProjectileSpawnOffset, 0.1f, 1.f, "%.2f");

	ImGui::InputText("ProjectileType", &myProjectileEntityPrefab);

	if (ImGui::BeginDragDropTarget())
	{
		ImGuiDragDropFlags target_flags = 0;
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EntityPrefab::GetAssetTypeName(), target_flags))
		{
			int assetIndex = *(const int*)payload->Data;
			const EntityPrefab* prefab = static_cast<EntityPrefab*>(ActionGameGlobals::GetInstance().GetEntityPrefabStorage().GetAllAssets()[assetIndex]);
			myProjectileEntityPrefab = prefab->GetAssetName();
		}
		ImGui::EndDragDropTarget();
	}
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

	const Data& shootingData = myEntityPrefab.GetProjectileShootingData();

	Vector2f projPosition = myEntity.myPosition + aDirection * shootingData.myProjectileSpawnOffset;
	Entity* projectile = myEntity.myEntityManager.CreateEntity(projPosition, shootingData.myProjectileEntityPrefab.GetBuffer());
	projectile->GetComponent<PhysicsComponent>()->myObject->myVelocity = aDirection * shootingData.myProjectileSpeed;
	projectile->GetComponent<SpriteComponent>()->GetSprite().SetRotation(FW_SignedAngle(aDirection));
}

void ProjectileShootingComponent::TriggerCooldown()
{
	myShootingReadyTimestamp = Slush::Time::GetCurrentExactTime() + Slush::Time::ConvertGameTimeToTimeUnit(myEntityPrefab.GetProjectileShootingData().myCooldown);
}
