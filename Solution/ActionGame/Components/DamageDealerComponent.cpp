#include "stdafx.h"

#include "DamageDealerComponent.h"

#include "HealthComponent.h"
#include "EntitySystem/Components/PhysicsComponent.h"
#include "CollisionFlag.h"
#include "ActionGameGlobals.h"
#include "EntitySystem\EntityManager.h"

void DamageDealerComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseIntField("damage", myDamage);
	aComponentHandle.ParseStringField("prefab", myImpactPrefab);
}

void DamageDealerComponent::Data::OnBuildUI()
{
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("Damage", &myDamage);

	ImGui::InputText("Prefab", &myImpactPrefab);

	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<Slush::EntityPrefab>()))
			myImpactPrefab = asset->GetAssetName();

		ImGui::EndDragDropTarget();
	}
}

//////////////////////////////////////////////////////////////////////////

DamageDealerComponent::DamageDealerComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	myDamage = myEntityPrefab.GetComponentData<DamageDealerComponent>().myDamage;
}

void DamageDealerComponent::OnCollision(Slush::Entity& aOtherEntity, const Vector2f& aContactPosition)
{
	Slush::PhysicsComponent* myPhysics = myEntity.GetComponent<Slush::PhysicsComponent>();
	Slush::PhysicsComponent* otherPhysics = aOtherEntity.GetComponent<Slush::PhysicsComponent>();
	if (!myPhysics || !otherPhysics)
		return;

	if (otherPhysics->GetCollisionFlag() == CollisionUtils::COL_ENVIRONMENT)
		return;

	HealthComponent* otherHealth = aOtherEntity.GetComponent<HealthComponent>();
	if (!otherHealth)
		return;

	if (myDamagedEntities.Contains(aOtherEntity.myHandle))
		return;

	myDamagedEntities.Add(aOtherEntity.myHandle);

	CollisionUtils::CollisionFlag myFlag = static_cast<CollisionUtils::CollisionFlag>(myPhysics->GetCollisionFlag());
	CollisionUtils::CollisionFlag otherFlag = static_cast<CollisionUtils::CollisionFlag>(otherPhysics->GetCollisionFlag());

	bool isPlayerSide = myFlag == CollisionUtils::COL_PLAYER || myFlag == CollisionUtils::COL_PLAYER_PROJECTILE;
	bool isNPCSide = myFlag == CollisionUtils::COL_NPC || myFlag == CollisionUtils::COL_NPC_PROJECTILE;
	bool otherIsPlayerSide = otherFlag == CollisionUtils::COL_PLAYER || otherFlag == CollisionUtils::COL_PLAYER_PROJECTILE;
	bool otherIsNPCSide = otherFlag == CollisionUtils::COL_NPC || otherFlag == CollisionUtils::COL_NPC_PROJECTILE;

	if ((isPlayerSide && otherIsNPCSide) || (isNPCSide && otherIsPlayerSide))
		otherHealth->DealDamage(myDamage);

	const Data& data = myEntityPrefab.GetComponentData<DamageDealerComponent>();
	if (!data.myImpactPrefab.Empty())
		ActionGameGlobals::GetInstance().GetEntityManager().CreateEntity(aContactPosition, data.myImpactPrefab.GetBuffer());
}