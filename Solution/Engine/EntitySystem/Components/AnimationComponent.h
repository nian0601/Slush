#pragma once

#include "EntitySystem/Component.h"
#include <FW_Vector2.h>

namespace Slush
{
	class Animation;
	class BaseSprite;

	struct AnimationRuntime;

	class AnimationComponent : public Component
	{
	public:
		COMPONENT_HELPER("Animation", "animation", 1);

		using Component::Component;
		~AnimationComponent();

		void Update() override;

		AnimationRuntime* PlayAnimation(const Animation& anAnimation, bool aOverrideSpriteSheetAnimation = true);
		bool IsAnimationPlaying(const Animation& anAnimation) const;

	private:
		struct RunningAnimation
		{
			const Animation* myAnimation = nullptr;
			AnimationRuntime* myRuntime = nullptr;
		};

		FW_GrowingArray<RunningAnimation> myRunningAnimations;
		void ApplyAnimation(AnimationRuntime& aRuntimeData);

		void InitRuntime(RunningAnimation& aRunningAnimation) const;
	};
}
