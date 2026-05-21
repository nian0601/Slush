#pragma once

#include "Component.h"

class RemoveOnCollisionComponent : public Component
{
public:
	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		FW_StaticArray<bool, EntityType::ENTITYTYPE_COUNT> myCollisionFlags;
	};
public:
	COMPONENT_HELPER("Remove On Collision", "removeoncollision");

	using Component::Component;

	void OnCollision(Entity& aOtherEntity, const Vector2f& aContactPosition) override;
};