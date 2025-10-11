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

	void Interpolator::OnParse(AssetParser::Handle aHandle)
	{
		AssetParser::Handle interpolatorHandle = aHandle.ParseChildElement("interpolator");
		interpolatorHandle.ParseFloatField("startvalue", myStartValue);
		interpolatorHandle.ParseFloatField("endvalue", myEndValue);

		int typeAsInt = myType;
		interpolatorHandle.ParseIntField("type", typeAsInt);
		myType = static_cast<Type>(typeAsInt);
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
			{
				outValue = myInterpolator.GetValue(1.f);
				OnUpdate();
			}

			return State::Finished;
		}

		if (!myIsWaitingClip)
		{
			float progress = FW_UnLerp(myStartTime, myEndTime, anElapsedTime);
			outValue = myInterpolator.GetValue(progress);
			OnUpdate();
		}

		return State::Running;
	}

	void AnimationClip::OnParse(AssetParser::Handle aHandle)
	{
		aHandle.ParseFloatField("starttime", myStartTime);
		aHandle.ParseFloatField("endtime", myEndTime);
		aHandle.ParseBoolField("iswaitingclip", myIsWaitingClip);
		myInterpolator.OnParse(aHandle);
	}


	void SpritesheetClip::OnParse(AssetParser::Handle aHandle)
	{
		AnimationClip::OnParse(aHandle);
		aHandle.ParseVec2iField("framepos", myFramePos);
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

	void AnimationTrack::OnParse(const char* aTrackName, AssetParser::Handle aHandle)
	{
		AssetParser::Handle trackHandle = aHandle.ParseChildElement(aTrackName);

		if (trackHandle.IsReading())
		{
			int numClips = trackHandle.GetNumChildElements();
			myClips.Reserve(numClips);
		}

		for (int i = 0; i < myClips.Count(); ++i)
		{
			AssetParser::Handle clipHandle;
			if (trackHandle.IsReading())
				clipHandle = trackHandle.GetChildElementAtIndex(i);
			else
				clipHandle = trackHandle.ParseChildElement("clip");

			AnimationClip& clip = myClips[i];
			
			clip.OnParse(clipHandle);
		}
		
	}

	AnimationClip& AnimationTrack::AddClip(float aDuration)
	{
		AnimationClip& clip = myClips.Add();
		clip.SetStartTimeAndDuration(myEndTime, aDuration);

		myEndTime += aDuration;

		return clip;
	}


	SpritesheetTrack& SpritesheetTrack::Frame(const Vector2i& aFramePosition)
	{
		SpritesheetClip& clip = AddClip(1.f / myFPS);
		clip.myFramePos = aFramePosition;

		return *this;
	}


	bool SpritesheetTrack::Update(float anElapsedTime, SpritesheetRuntimeTrackData& aTrackData)
	{
		if (aTrackData.myCurrentClip >= myClips.Count())
		{
			aTrackData.myIsActive = false;
			return false;
		}

		AnimationClip::State state = myClips[aTrackData.myCurrentClip].Update(anElapsedTime, aTrackData.myValue);

		Vector2i framePos = myClips[aTrackData.myCurrentClip].myFramePos;
		aTrackData.myFrameRect = MakeRectFromTopLeft(framePos * myFrameSize, myFrameSize);

		if (state == AnimationClip::State::Finished)
			++aTrackData.myCurrentClip;

		aTrackData.myIsActive = state != AnimationClip::State::NotStarted;
		return true;
	}

	void SpritesheetTrack::OnParse(const char* aTrackName, AssetParser::Handle aHandle)
	{
		AssetParser::Handle trackHandle = aHandle.ParseChildElement(aTrackName);

		trackHandle.ParseFloatField("fps", myFPS);
		trackHandle.ParseVec2iField("framesize", myFrameSize);

		if (trackHandle.IsReading())
		{
			// Do -1 because of the 'framesize'-child
			int numClips = trackHandle.GetNumChildElements() - 1;
			myClips.Reserve(numClips);
		}

		for (int i = 0; i < myClips.Count(); ++i)
		{
			AssetParser::Handle clipHandle;
			if (trackHandle.IsReading())
				clipHandle = trackHandle.GetChildElementAtIndex(i+1);
			else
				clipHandle = trackHandle.ParseChildElement("clip");

			SpritesheetClip& clip = myClips[i];

			clip.OnParse(clipHandle);
		}
	}

	SpritesheetClip& SpritesheetTrack::AddClip(float aDuration)
	{
		SpritesheetClip& clip = myClips.Add();
		clip.SetStartTimeAndDuration(myEndTime, aDuration);
		myEndTime += aDuration;

		return clip;
	}

	//////////////////////////////////////////////////////////////////////////

	Animation::Animation(const char* aName)
		: DataAsset(aName)
	{
	}

	void Animation::OnParse(AssetParser::Handle aRootHandle)
	{
		myOutlineTrack.OnParse("outlinetrack", aRootHandle);
		myScaleTrack.OnParse("scaletrack", aRootHandle);
		myPositionTrack.OnParse("postiontrack", aRootHandle);
		myColorTrack.OnParse("colortrack", aRootHandle);
		mySpritesheetTrack.OnParse("spritesheettrack", aRootHandle);
	}

	void Animation::BuildUI()
	{

	}

	void Animation::Update(AnimationRuntime& aRuntimeData)
	{
		if (aRuntimeData.myState != AnimationClip::Running)
			return;

		aRuntimeData.myElapsedTime += Time::GetDelta();

		bool anyTrackActive = false;
		anyTrackActive |= myOutlineTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myOutlineData);
		anyTrackActive |= myScaleTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myScaleData);
		anyTrackActive |= myPositionTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myPositionData);
		anyTrackActive |= myColorTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myColorData);
		anyTrackActive |= mySpritesheetTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.mySpritesheetData);

		if (!anyTrackActive)
			aRuntimeData.myState = AnimationRuntime::Finished;
	}
}