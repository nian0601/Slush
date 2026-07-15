#pragma once

#include "EntitySystem/Component.h"
#include "EntitySystem/Components/PhysicsComponent.h"
#include "CollisionFlag.h"

class RemoveOnCollisionComponent : public Slush::Component
{
public:
	struct Data : public Slush::Component::BaseData
	{
		Data() { myCollisionFlags.Fill(false); }

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		FW_StaticArray<bool, CollisionUtils::COLLISIONFLAG_COUNT> myCollisionFlags;
	};
public:
	COMPONENT_HELPER("Remove On Collision", "removeoncollision");

	using Slush::Component::Component;

	void OnCollision(Slush::Entity& aOtherEntity, const Vector2f& aContactPosition) override;
};