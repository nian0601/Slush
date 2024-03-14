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
	AnimationComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
	~AnimationComponent();

	void Update();
	bool AnimationIsPlaying() const;

	void PlayDash(const Vector2f& aTargetPosition);

private:
	Slush::Animation* myDashAnimation = nullptr;
	Slush::AnimationRuntime* myRuntime = nullptr;
};
