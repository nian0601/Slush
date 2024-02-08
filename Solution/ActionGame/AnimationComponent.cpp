#include "AnimationComponent.h"

#include "Entity.h"
#include "SpriteComponent.h"

#include <Graphics\Animation\Animation.h>
#include <Graphics\Animation\AnimationRuntime.h>

#include <Core\Log.h>
#include <FW_Includes.h>

AnimationComponent::AnimationComponent(Entity& anEntity)
	: Component(anEntity)
{
	myDashAnimation = new Slush::Animation();
	myDashAnimation->MakeOneShot();
	myDashAnimation->myScaleTrack
		.Linear(0.1f, 1.f, 0.2f)
		.Wait(0.1f)
		.Linear(0.1f, 0.2f, 1.f);
	myDashAnimation->myPositionTrack
		.Wait(0.1f)
		.Linear(0.25f, 0.f, 1.f);

	myRuntime = new Slush::AnimationRuntime();
}

AnimationComponent::~AnimationComponent()
{
	FW_SAFE_DELETE(myDashAnimation);
	FW_SAFE_DELETE(myRuntime);
}

void AnimationComponent::Update()
{
	if (!myEntity.mySpriteComponent)
	{
		SLUSH_WARNING("Updating AnimationComponent without a SpriteComponent");
		return;
	}

	myDashAnimation->Update(*myRuntime, myEntity.mySpriteComponent->GetSprite());
	if (myRuntime->myPositionData.myIsActive)
		myEntity.myPosition = myRuntime->myCurrentPosition;
}

bool AnimationComponent::AnimationIsPlaying() const
{
	return myRuntime->myState == Slush::AnimationClip::Running;
}

void AnimationComponent::PlayDash(const Vector2f& aTargetPosition)
{
	myRuntime->myStartPosition = myEntity.myPosition;
	myRuntime->myEndPosition = aTargetPosition;
	myRuntime->Start();
}
