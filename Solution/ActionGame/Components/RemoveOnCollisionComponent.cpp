#include "stdafx.h"

#include "RemoveOnCollisionComponent.h"

#include <Physics\PhysicsWorld.h>

void RemoveOnCollisionComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	const char* const* serializationNames = CollisionUtils::GetSerializationNames();
	for (int i = 0; i < CollisionUtils::COLLISIONFLAG_COUNT; ++i)
		aComponentHandle.ParseBoolField(serializationNames[i], myCollisionFlags[i]);
}

void RemoveOnCollisionComponent::Data::OnBuildUI()
{
	ImGui::Text("Remove when colliding with:");
	const char* const* names = CollisionUtils::GetNames();
	for (int i = 0; i < CollisionUtils::COLLISIONFLAG_COUNT; ++i)
	{
		ImGui::Checkbox(names[i], &myCollisionFlags[i]);
	}
}

void RemoveOnCollisionComponent::OnCollision(Slush::Entity& aOtherEntity, const Vector2f& /*aContactPosition*/)
{
	Slush::PhysicsComponent* otherPhysics = aOtherEntity.GetComponent<Slush::PhysicsComponent>();
	if (!otherPhysics)
		return;

	const RemoveOnCollisionComponent::Data& data = myEntityPrefab.GetComponentData<RemoveOnCollisionComponent>();

	for (int i = 0; i < CollisionUtils::COLLISIONFLAG_COUNT; ++i)
	{
		if (data.myCollisionFlags[i] && (otherPhysics->myObject->myCollisionMask & (1u << i)))
		{
			myEntity.myIsMarkedForRemoval = true;
			return;
		}
	}
}