#pragma once

#include <FW_Vector2.h>

#include "EntityHandle.h"
#include "Component.h"
#include <FW_TypeID.h>
#include "EntityPrefab.h"

namespace Slush
{
	class PhysicsWorld;
	class EntityManager;

	class Entity
	{
		friend class EntityManager;
	public:
		~Entity();

		void OnEnterWorld();
		void PrePhysicsUpdate();
		void Update();
		void Render();

		void OnCollision(Entity& aOtherEntity, const Vector2f& aContactPosition);
		void OnDeath();

		template <typename ComponentType>
		ComponentType* GetComponent();

		template <typename ComponentType>
		const ComponentType* GetComponent() const;


		Vector2f myPosition;
		bool myIsMarkedForRemoval = false;
		EntityHandle myHandle;
		EntityManager& myEntityManager;

	private:
		Entity(EntityManager& aEntityManager);

		// Will contain nullptrs, but Components are stored sorted by TypeID for easy access
		FW_StaticArray<Component*, 32> myComponents;

		// Guaranteed to not contain any nullptrs, but will not store components sorted by TypeID
		FW_GrowingArray<Component*> myPackedComponents;

		void CreateComponents(const EntityPrefab& aPrefab);
	};

	template <typename ComponentType>
	inline ComponentType* Entity::GetComponent()
	{
		unsigned int id = FW_TypeID<Component>::GetID<ComponentType>();
		FW_ASSERT(id < 32, "Too many ComponentTypes");
		return static_cast<ComponentType*>(myComponents[id]);
	}

	template <typename ComponentType>
	inline const ComponentType* Entity::GetComponent() const
	{
		unsigned int id = FW_TypeID<Component>::GetID<ComponentType>();
		FW_ASSERT(id < 32, "Too many ComponentTypes");
		return static_cast<const ComponentType*>(myComponents[id]);
	}

}
