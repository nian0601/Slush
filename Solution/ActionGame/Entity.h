#pragma once

#include <FW_Vector2.h>

#include "EntityHandle.h"

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
	};

	void PrePhysicsUpdate();
	void Update();
	void Render();

	void OnCollision(Entity& aOtherEntity);

	SpriteComponent* mySpriteComponent = nullptr;
	AnimationComponent* myAnimationComponent = nullptr;
	ProjectileShootingComponent* myProjectileShootingComponent = nullptr;
	PlayerControllerComponent* myPlayerControllerComponent = nullptr;
	NPCControllerComponent* myNPCControllerComponent = nullptr;
	HealthComponent* myHealthComponent = nullptr;
	PhysicsComponent* myPhysicsComponent = nullptr;
	RemoveOnCollisionComponent* myRemoveOnCollisionComponent = nullptr;
	TargetingComponent* myTargetingComponent = nullptr;
	WeaponComponent* myWeaponComponent = nullptr;
	ExperienceComponent* myExperienceComponent = nullptr;

	Vector2f myPosition;
	Type myType = ENVIRONMENT;
	bool myIsMarkedForRemoval = false;
	EntityHandle myHandle;
	EntityManager& myEntityManager;

private:
	Entity(EntityManager& aEntityManager);

	void CreateComponents(const EntityPrefab& aPrefab, Slush::PhysicsWorld& aPhysicsWorld);
};
