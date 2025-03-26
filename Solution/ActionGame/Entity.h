#pragma once

#include <FW_Vector2.h>

#include "EntityHandle.h"
#include "Component.h"
#include <FW_TypeID.h>
#include "EntityPrefab.h"

class SpriteComponent;
class AnimationComponent;
class ProjectileShootingComponent;
class PlayerControllerComponent;
class NPCControllerComponent;
class HealthComponent;
class PhysicsComponent;
class RemoveOnCollisionComponent;
class TargetingComponent;
class WeaponComponent;
class ExperienceComponent;
class PickupComponent;
class StatsComponent;
class DamageDealerComponent;
class HealthBarComponent;

class EntityManager;

namespace Slush
{
	class PhysicsWorld;
}

class Entity
{
	friend class EntityManager;
public:
	~Entity();

	void PrePhysicsUpdate();
	void Update();
	void Render();

	void OnCollision(Entity& aOtherEntity);
	void OnDeath();

	bool IsPlayerOwned() const { return myType == PLAYER || myType == PLAYER_PROJECTILE; }
	bool IsNPCOwned() const { return myType == NPC || myType == NPC_PROJECTILE; }

	template <typename ComponentType>
	ComponentType* GetComponent();

	template <typename ComponentType>
	const ComponentType* GetComponent() const;


	Vector2f myPosition;
	EntityType myType = ENVIRONMENT;
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

	template <typename ComponentType>
	void CreateComponent(const EntityPrefab& aPrefab);
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

template <typename ComponentType>
inline void Entity::CreateComponent(const EntityPrefab& aPrefab)
{
	if (aPrefab.Has<ComponentType>())
	{
		int index = GetComponentID<ComponentType>();
		myComponents[index] = new ComponentType(*this, aPrefab);
		myPackedComponents.Add(myComponents[index]);
	}
}