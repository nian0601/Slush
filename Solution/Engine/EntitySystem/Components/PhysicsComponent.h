#pragma once

#include "EntitySystem/Component.h"

#include <FW_Vector2.h>

namespace Slush
{
	class PhysicsWorld;

	struct PhysicsObject;

	class PhysicsComponent : public Component
	{
	public:
		struct Data : public Component::BaseData
		{
			void OnParse(AssetParser::Handle aComponentHandle) override;
			void OnBuildUI() override;

			virtual void OnParseCollisionFlags(AssetParser::Handle aComponentHandle) { aComponentHandle; }
			virtual void OnBuildCollisionFlagsUI() {}

			bool myStatic = false;
			bool mySensor = false;
			bool myMatchSprite = true;
			float myRadius = 10.f;
			Vector2f mySize;

			int myCollisionFlag = 0;
			unsigned int myCollidesWithMask = 0;
		};

	public:
		COMPONENT_HELPER("Physics", "physics");

		PhysicsComponent(Entity& aEntity, const EntityPrefab& anEntityPrefab);
		~PhysicsComponent();

		void Update() override;

		int GetCollisionFlag() const;

		PhysicsObject* myObject;

	private:
		PhysicsWorld& myPhysicsWorld;
	};
}
