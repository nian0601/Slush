#pragma once

#include "Component.h"
#include <FW_Vector2.h>

namespace Slush
{
	class Animation;
	
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
	bool AnimationIsPlaying() const;

	void PlayDash(const Vector2f& aTargetPosition);
	void PlayBlink();
	void PlaySpawn();
	void PlaySpritesheetAnimation();

	bool IsPlayingSpawn() const { return myCurrentAnimation == mySpawnAnimation; }

private:
	Slush::Animation* myDashAnimation = nullptr;
	Slush::Animation* myBlinkAnimation = nullptr;
	Slush::Animation* mySpawnAnimation = nullptr;
	Slush::Animation* mySpritesheetAnimation = nullptr;

	Slush::Animation* myCurrentAnimation = nullptr;
	Slush::AnimationRuntime* myRuntime = nullptr;
};
