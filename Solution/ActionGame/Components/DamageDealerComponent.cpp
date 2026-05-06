#include "stdafx.h"

#include "DamageDealerComponent.h"

#include "HealthComponent.h"
#include "ActionGameGlobals.h"
#include "EntitySystem\EntityManager.h"

void DamageDealerComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
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
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<EntityPrefab>()))
			myImpactPrefab = asset->GetAssetName();

		ImGui::EndDragDropTarget();
	}
}

//////////////////////////////////////////////////////////////////////////

DamageDealerComponent::DamageDealerComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	myDamage = myEntityPrefab.GetDamageDealerData().myDamage;
}

void DamageDealerComponent::OnCollision(Entity& aOtherEntity, const Vector2f& aContactPosition)
{
	EntityType otherType = aOtherEntity.myType;
	if (otherType == EntityType::ENVIRONMENT)
		return;

	HealthComponent* otherHealth = aOtherEntity.GetComponent<HealthComponent>();
	if (!otherHealth)
		return;

	if (myEntity.IsPlayerOwned() && aOtherEntity.IsNPCOwned())
		otherHealth->DealDamage(myDamage);
	else if (myEntity.IsNPCOwned() && aOtherEntity.IsPlayerOwned())
		otherHealth->DealDamage(myDamage);

	const Data& data = myEntityPrefab.GetDamageDealerData();
	if (!data.myImpactPrefab.Empty())
		ActionGameGlobals::GetInstance().GetEntityManager().CreateEntity(aContactPosition, data.myImpactPrefab.GetBuffer());
}