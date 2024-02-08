#pragma once

#include <FW_Vector2.h>

class SpriteComponent;
class AnimationComponent;
class ProjectileShootingComponent;
class PlayerControllerComponent;
class NPCControllerComponent;

class Entity
{
public:
	~Entity();

	void Update();
	void Render();

	SpriteComponent* mySpriteComponent = nullptr;
	AnimationComponent* myAnimationComponent = nullptr;
	ProjectileShootingComponent* myProjectileShootingComponent = nullptr;
	PlayerControllerComponent* myPlayerControllerComponent = nullptr;
	NPCControllerComponent* myNPCControllerComponent = nullptr;

	Vector2f myPosition;
};
