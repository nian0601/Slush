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

	void PlayDash(const Vector2f& aTargetPosition);
	void PlayBlink();
	void PlaySpritesheetAnimation();


	Slush::AnimationRuntime* PlayAnimation(const Slush::Animation& anAnimation);
	bool IsAnimationPlaying(const Slush::Animation& anAnimation) const;

private:
	struct RunningAnimation
	{
		const Slush::Animation* myAnimation = nullptr;
		Slush::AnimationRuntime* myRuntime = nullptr;
	};

	FW_GrowingArray<RunningAnimation> myRunningAnimations;
	void ApplyAnimation(Slush::AnimationRuntime& aRuntimeData);

	Slush::Animation* myDashAnimation = nullptr;
	Slush::Animation* myBlinkAnimation = nullptr;
	Slush::Animation* mySpritesheetAnimation = nullptr;
};
