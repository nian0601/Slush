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

	void AnimationRuntime::Restart()
	{
		myState = Running;
		myElapsedTime = 0.f;

		myOutlineData.Reset();
		myOutlineData.myValue = 1.f;
		myScaleData.Reset();
		myScaleData.myValue = 1.f;
		myPositionData.Reset();
	}
}