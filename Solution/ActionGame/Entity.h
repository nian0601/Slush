#pragma once

#include <Graphics\Animation\AnimationRuntime.h>
#include <FW_Vector2.h>

namespace Slush
{
	class BaseSprite;
	class Animation;
}

class ProjectileManager;

class Entity
{
public:
	Entity();
	~Entity();

	void Update(ProjectileManager& aProjectileManager);
	void Render();

	Slush::BaseSprite* mySprite = nullptr;
	Slush::Animation* myAnimation = nullptr;
	Slush::AnimationRuntime myAnimationRuntime;

	Vector2f myPosition;
	Vector2f myDirection;
	float mySpeed;
};
