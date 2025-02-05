#include "stdafx.h"

#include "Graphics/Animation/Animation.h"
#include "Graphics/Animation/AnimationRuntime.h"
#include "Graphics/BaseSprite.h"
#include "Core/Time.h"

namespace Slush
{
	float Interpolator::GetValue(float aProgress)
	{
		if (myType == Linear)
			return FW_Lerp(myStartValue, myEndValue, aProgress);
		else if (myType == Constant)
			return myStartValue;

		return 0.f;
	}

	void Interpolator::MakeLinear(float aStart, float aEnd)
	{
		myType = Linear;
		myStartValue = aStart;
		myEndValue = aEnd;
	}

	void Interpolator::MakeConstant(float aValue)
	{
		myType = Constant;
		myStartValue = aValue;
		myEndValue = aValue;
	}

	//////////////////////////////////////////////////////////////////////////

	void AnimationClip::SetStartTimeAndDuration(float aStartTime, float aDuration)
	{
		myStartTime = aStartTime;
		myEndTime = myStartTime + aDuration;
	}

	AnimationClip::State AnimationClip::Update(float anElapsedTime, float& outValue)
	{
		if (anElapsedTime < myStartTime)
			return State::NotStarted;

		if (anElapsedTime >= myEndTime)
		{
			if (!myIsWaitingClip)
				outValue = myInterpolator.GetValue(1.f);

			return State::Finished;
		}

		if (!myIsWaitingClip)
		{
			float progress = FW_UnLerp(myStartTime, myEndTime, anElapsedTime);
			outValue = myInterpolator.GetValue(progress);
		}

		return State::Running;
	}

	//////////////////////////////////////////////////////////////////////////

	AnimationTrack& AnimationTrack::Linear(float aDuration, float aStart, float aEnd)
	{
		AnimationClip& clip = AddClip(aDuration);
		clip.myInterpolator.MakeLinear(aStart, aEnd);

		return *this;
	}

	AnimationTrack& AnimationTrack::Constant(float aDuration, float aValue)
	{
		AnimationClip& clip = AddClip(aDuration);
		clip.myInterpolator.MakeConstant(aValue);

		return *this;
	}

	AnimationTrack& AnimationTrack::Wait(float aDuration)
	{
		AnimationClip& clip = AddClip(aDuration);
		clip.MakeWaitingClip();

		return *this;
	}

	bool AnimationTrack::Update(float anElapsedTime, AnimationRuntimeTrackData& aTrackData)
	{
		if (aTrackData.myCurrentClip >= myClips.Count())
		{
			aTrackData.myIsActive = false;
			return false;
		}

		AnimationClip::State state = myClips[aTrackData.myCurrentClip].Update(anElapsedTime, aTrackData.myValue);
		if (state == AnimationClip::State::Finished)
			++aTrackData.myCurrentClip;

		aTrackData.myIsActive = state != AnimationClip::State::NotStarted;
		return true;
	}

	Slush::AnimationClip& AnimationTrack::AddClip(float aDuration)
	{
		AnimationClip& clip = myClips.Add();
		clip.SetStartTimeAndDuration(myEndTime, aDuration);

		myEndTime += aDuration;

		return clip;
	}

	//////////////////////////////////////////////////////////////////////////
	void Animation::Update(AnimationRuntime& aRuntimeData, BaseSprite& aSprite)
	{
		if (aRuntimeData.myState != AnimationClip::Running)
			return;

		aRuntimeData.myElapsedTime += Time::GetDelta();

		bool anyTrackActive = false;
		anyTrackActive |= myOutlineTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myOutlineData);
		anyTrackActive |= myScaleTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myScaleData);
		anyTrackActive |= myPositionTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myPositionData);
		anyTrackActive |= myColorTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myColorData);

		if (!anyTrackActive)
			aRuntimeData.myState = AnimationRuntime::Finished;

		ApplyAnimation(aRuntimeData, aSprite);

		if (aRuntimeData.myState == AnimationRuntime::Finished && myIsLooping)
			aRuntimeData.Start();
	}

	void Animation::ApplyAnimation(AnimationRuntime& aRuntimeData, BaseSprite& aSprite)
	{
		if (aRuntimeData.myOutlineData.myIsActive)
			aSprite.SetOutlineThickness(aRuntimeData.myOutlineData.myValue);

		if (aRuntimeData.myScaleData.myIsActive)
			aSprite.SetScale(aRuntimeData.myScaleData.myValue);

		if (aRuntimeData.myPositionData.myIsActive)
			aRuntimeData.myCurrentPosition = FW_Lerp(aRuntimeData.myStartPosition, aRuntimeData.myEndPosition, aRuntimeData.myPositionData.myValue);

		if (aRuntimeData.myColorData.myIsActive)
			aSprite.SetFillColor(FW_Interpolate_Color(aRuntimeData.myStartColor, aRuntimeData.myEndColor, aRuntimeData.myColorData.myValue));
	}
}