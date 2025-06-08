#include "stdafx.h"

#include "Graphics/Animation/AnimationRuntime.h"

namespace Slush
{
	void AnimationRuntimeTrackData::Reset()
	{
		myIsActive = false;
		myValue = 0.f;
		myCurrentClip = 0;
	}

	void AnimationRuntime::Start()
	{
		myState = Running;
		myElapsedTime = 0.f;

		myOutlineData.Reset();
		myOutlineData.myValue = 1.f;
		myScaleData.Reset();
		myScaleData.myValue = 1.f;
		myPositionData.Reset();
		myColorData.Reset();
		mySpritesheetData.Reset();
	}

	void SpritesheetRuntimeTrackData::Reset()
	{
		AnimationRuntimeTrackData::Reset();

		myFrameRect = MakeRect(0, 0, 0, 0);
	}

}