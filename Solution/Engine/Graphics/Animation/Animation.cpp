#include "stdafx.h"

#include "Graphics/Animation/Animation.h"
#include "Graphics/BaseSprite.h"
#include "Core/Time.h"

namespace Slush
{

	float Interpolator::GetValue(float aProgress)
	{
		if (myType == Linear)
			return FW_Lerp(myStartValue, myEndValue, aProgress);

		return 0.f;
	}

	void Interpolator::MakeLinear(float aStart, float aEnd)
	{
		myType = Linear;
		myStartValue = aStart;
		myEndValue = aEnd;
	}

	//////////////////////////////////////////////////////////////////////////

	void AnimationClip::SetStartTimeAndDuration(float aStartTime, float aDuration)
	{
		myStartTime = aStartTime;
		myEndTime = myStartTime + aDuration;
	}

	float AnimationClip::Update(float anElapsedTime)
	{
		if (myState == Waiting && anElapsedTime >= myStartTime)
			myState = Running;

		if (myState == Running)
		{
			float progress = FW_UnLerp(myStartTime, myEndTime, anElapsedTime);
			float value = myInterpolator.GetValue(progress);

			if (anElapsedTime >= myEndTime)
				myState = Finished;
			else
				return value;
		}

		if (myState == Finished)
			return myInterpolator.GetValue(1.f);

		return FLT_MAX;
	}

	//////////////////////////////////////////////////////////////////////////

	AnimationTrack::AnimationTrack(Animation& anParentAnimation, TrackType aTrackType)
		: myParentAnimation(anParentAnimation)
		, myTrackType(aTrackType)
	{
	}

	AnimationTrack& AnimationTrack::Linear(float aDuration, float aStart, float aEnd)
	{
		AnimationClip& clip = myClips.Add();
		clip.SetStartTimeAndDuration(myEndTime, aDuration);
		clip.myInterpolator.MakeLinear(aStart, aEnd);

		myEndTime += aDuration;
		myParentAnimation.myTotalTime = FW_Max(myParentAnimation.myTotalTime, myEndTime);
		
		return *this;
	}

	AnimationTrack& AnimationTrack::Wait(float aDuration)
	{
		myEndTime += aDuration;
		myParentAnimation.myTotalTime = FW_Max(myParentAnimation.myTotalTime, myEndTime);

		return *this;
	}

	void AnimationTrack::Update(float anElapsedTime, BaseSprite& aSprite)
	{
		for (AnimationClip& clip : myClips)
		{
			float animValue = clip.Update(anElapsedTime);
			if (animValue < FLT_MAX)
				ApplyAnimation(animValue, aSprite);
		}
	}

	void AnimationTrack::Reset()
	{
		const float elaspedTime = 0.f;
		for (AnimationClip& clip : myClips)
		{
			clip.myState = AnimationClip::Waiting;
			clip.Update(elaspedTime);
		}
	}

	void AnimationTrack::ApplyAnimation(float aValue, BaseSprite& aSprite)
	{
		switch (myTrackType)
		{
		case TrackType::Outline:
			aSprite.SetOutlineThickness(aValue);
			break;
		case TrackType::Scale:
			aSprite.SetScale(aValue);
			break;
		default:
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	Animation::Animation(BaseSprite& aSprite)
		: mySprite(aSprite)
		, myOutlineTrack(*this, AnimationTrack::Outline)
		, myScaleTrack(*this, AnimationTrack::Scale)
	{
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
		}

		myOutlineTrack.Update(myElapsedTime, mySprite);
		myScaleTrack.Update(myElapsedTime, mySprite);

		if (myState == Finished && myIsLooping)
			Restart();
	}

	void Animation::Restart()
	{
		myState = Running;
		myElapsedTime = 0.f;

		myOutlineTrack.Reset();
		myScaleTrack.Reset();
	}
}