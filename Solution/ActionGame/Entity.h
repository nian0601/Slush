#pragma once

#include <FW_Vector2.h>

#include "EntityHandle.h"

class SpriteComponent;
class AnimationComponent;
class ProjectileShootingComponent;
class PlayerControllerComponent;
class NPCControllerComponent;
class CollisionComponent;
class HealthComponent;
class PhysicsComponent;

class Entity
{
public:
	enum Type
	{
		PLAYER,
		NPC,
	};

	~Entity();

	void PrePhysicsUpdate();
	void Update();
	void Render();

	SpriteComponent* mySpriteComponent = nullptr;
	AnimationComponent* myAnimationComponent = nullptr;
	ProjectileShootingComponent* myProjectileShootingComponent = nullptr;
	PlayerControllerComponent* myPlayerControllerComponent = nullptr;
	NPCControllerComponent* myNPCControllerComponent = nullptr;
	CollisionComponent* myCollisionComponent = nullptr;
	HealthComponent* myHealthComponent = nullptr;
	PhysicsComponent* myPhysicsComponent = nullptr;

	Vector2f myPosition;
	Type myType;
	bool myIsMarkedForRemoval = false;
	EntityHandle myHandle;
};
