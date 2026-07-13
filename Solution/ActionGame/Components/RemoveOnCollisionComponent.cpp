#include "stdafx.h"

#include "RemoveOnCollisionComponent.h"

void RemoveOnCollisionComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	const char* entityTypeNames[] = { "environment", "player", "npc", "playerprojectile", "npcprojectile", "pickup" };
	static_assert(IM_ARRAYSIZE(entityTypeNames) == EntityType::ENTITYTYPE_COUNT);

	for (int i = 0; i < IM_ARRAYSIZE(entityTypeNames); ++i)
		aComponentHandle.ParseBoolField(entityTypeNames[i], myCollisionFlags[i]);
}

void RemoveOnCollisionComponent::Data::OnBuildUI()
{
	ImGui::Text("Remove when colliding with:");
	for (int i = 0; i < EntityType::ENTITYTYPE_COUNT; ++i)
	{
		ImGui::Checkbox(GetEntityTypeNames()[i], &myCollisionFlags[i]);
	}
}

void RemoveOnCollisionComponent::OnCollision(Entity& aOtherEntity, const Vector2f& /*aContactPosition*/)
{
	const RemoveOnCollisionComponent::Data& data = myEntityPrefab.GetComponentData<RemoveOnCollisionComponent>();
	if (!data.myCollisionFlags[aOtherEntity.myType])
		return;

	myEntity.myIsMarkedForRemoval = true;
}