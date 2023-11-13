#include "stdafx.h"

#include "Graphics/Animation/Animation.h"
#include "Graphics/BaseSprite.h"
#include "Core/Time.h"

namespace Slush
{

	void AnimationClip::Update(float anElapsedTime, BaseSprite& aSprite)
	{
		if (myState == Waiting && anElapsedTime >= myStartTime)
		{
			myState = Running;
		}

		if (myState == Running)
		{
			float progress = FW_UnLerp(myStartTime, myEndTime, anElapsedTime);
			float value = FW_Lerp(myStartValue, myEndValue, progress);

			aSprite.SetOutlineThickness(value);

			if (anElapsedTime >= myEndTime)
				myState = Finished;
		}

		if (myState == Finished)
		{
			aSprite.SetOutlineThickness(myEndValue);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	Animation::Animation(BaseSprite& aSprite)
		: mySprite(aSprite)
	{
	}

	void Animation::AddClip(const AnimationClip& aClip)
	{
		myClips.Add(aClip);
		myTotalTime = FW_Max(myTotalTime, aClip.myEndTime);
	}

	void Animation::Update()
	{
		if (myState == Waiting || myState == Finished)
			return;

		myElapsedTime += Time::GetDelta();

		if (myElapsedTime >= myTotalTime)
		{
			myElapsedTime = myTotalTime;
			myState = Finished;

			if (myIsLooping)
				Restart();
		}

		for (AnimationClip& clip : myClips)
			clip.Update(myElapsedTime, mySprite);

	}

	void Animation::Restart()
	{
		myState = Running;
		myElapsedTime = 0.f;

		for (AnimationClip& clip : myClips)
		{
			clip.myState = AnimationClip::Waiting;
			clip.Update(myElapsedTime, mySprite);
		}
	}

}