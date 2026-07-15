#pragma once

#include <FW_GrowingArray.h>

#include "EntityHandle.h"
#include "Entity.h"
#include <FW_Vector2.h>
#include <Core/Assets/AssetStorage.h>

namespace Slush
{
	class PhysicsWorld;

	class EntityManager
	{
	public:
		static void RegisterComponents();

		EntityManager();
		~EntityManager();

		Entity* CreateEmptyEntity();
		Entity* CreateEntity(const Vector2f& aPosition, const EntityPrefab& aPrefab);
		Entity* CreateEntity(const Vector2f& aPosition, const char* aPrefabName);
		Entity* CreateEntity(const Vector2f& aPosition, const FW_String& aPrefabName);

		void DeleteAllEntities();

		void GetAllEntities(FW_GrowingArray<EntityHandle>& outEntityHandles) const;

		void SetPhysicsWorld(PhysicsWorld* aPhysicsWorld) { myPhysicsWorld = aPhysicsWorld; }
		PhysicsWorld& GetPhysicsWorld() const;

		void PrePhysicsUpdate();

		void Update();
		void Render();

		void EndFrame();

	private:
		void CleanupProxyStorage();

		FW_GrowingArray<EntityHandle::ProxyObject*> myProxyStorage;
		FW_GrowingArray<Entity*> myEntities;

		FW_GrowingArray<Entity*> myAddQueue;

		PhysicsWorld* myPhysicsWorld = nullptr;
	};
}
