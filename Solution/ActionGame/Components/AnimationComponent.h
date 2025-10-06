#pragma once

#include "Component.h"
#include <FW_Vector2.h>

namespace Slush
{
	class Animation;
	class BaseSprite;
	
	struct AnimationRuntime;
}

class EntityPrefab;
class AnimationComponent : public Component
{
public:
	COMPONENT_HELPER("Animation", "animation");

	AnimationComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
	~AnimationComponent();

	void OnEnterWorld() override;
	void Update() override;
	bool IsPlayingDash() const;
	bool IsPlayingSpawn() const;

	void PlayDash(const Vector2f& aTargetPosition);
	void PlayBlink();
	void PlaySpawn();
	void PlaySpritesheetAnimation();

private:
	struct RunningAnimation
	{
		Slush::Animation* myAnimation = nullptr;
		Slush::AnimationRuntime* myRuntime = nullptr;
	};

	FW_GrowingArray<RunningAnimation> myRunningAnimations;

	Slush::Animation* myDashAnimation = nullptr;
	Slush::Animation* myBlinkAnimation = nullptr;
	Slush::Animation* mySpawnAnimation = nullptr;
	Slush::Animation* mySpritesheetAnimation = nullptr;

	//Slush::Animation* myCurrentAnimation = nullptr;
	//Slush::AnimationRuntime* myRuntime = nullptr;
};
