#pragma once

#include "EntitySystem/Component.h"
#include "EntitySystem/EntityType.h"

class RemoveOnCollisionComponent : public Slush::Component
{
public:
	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		FW_StaticArray<bool, EntityType::ENTITYTYPE_COUNT> myCollisionFlags;
	};
public:
	COMPONENT_HELPER("Remove On Collision", "removeoncollision");

	using Slush::Component::Component;

	void OnCollision(Slush::Entity& aOtherEntity, const Vector2f& aContactPosition) override;
};