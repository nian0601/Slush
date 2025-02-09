#pragma once

#include <FW_Vector2.h>

#include "EntityHandle.h"
#include "Component.h"
#include <FW_TypeID.h>

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
class EntityPrefab;

namespace Slush
{
	class PhysicsWorld;
}

class Entity
{
	friend class EntityManager;
public:
	~Entity();

	enum Type
	{
		ENVIRONMENT,
		PLAYER,
		NPC,
		PLAYER_PROJECTILE,
		NPC_PROJECTILE,
		PICKUP,
	};

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

	RemoveOnCollisionComponent* myRemoveOnCollisionComponent = nullptr;
	TargetingComponent* myTargetingComponent = nullptr;
	WeaponComponent* myWeaponComponent = nullptr;
	ExperienceComponent* myExperienceComponent = nullptr;
	PickupComponent* myPickupComponent = nullptr;
	StatsComponent* myStatsComponent = nullptr;
	DamageDealerComponent* myDamageDealerComponent = nullptr;
	HealthBarComponent* myHealthBarComponent = nullptr;

	FW_StaticArray<Component*, 32> myComponents;

	Vector2f myPosition;
	Type myType = ENVIRONMENT;
	bool myIsMarkedForRemoval = false;
	EntityHandle myHandle;
	EntityManager& myEntityManager;

private:
	Entity(EntityManager& aEntityManager);

	// Guaranteed to not contain any nullptrs, but will not store components sorted by TypeID
	FW_GrowingArray<Component*> myPackedComponents;

	void CreateComponents(const EntityPrefab& aPrefab, Slush::PhysicsWorld& aPhysicsWorld);
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