#pragma once
#include <FW_Vector2.h>
#include <float.h>

namespace Slush
{
	struct AnimationRuntimeTrackData
	{
		void Reset();

		bool myIsActive = false;
		float myValue = FLT_MAX;
		int myCurrentClip = 0;
	};

	struct AnimationRuntime
	{
		enum State
		{
			Waiting,
			Running,
			Finished,
		};

		void Restart();

		State myState = Waiting;
		float myElapsedTime = 0.f;

		AnimationRuntimeTrackData myOutlineData;
		AnimationRuntimeTrackData myScaleData;
		AnimationRuntimeTrackData myPositionData;

		Vector2f myStartPosition;
		Vector2f myEndPosition;
		Vector2f myCurrentPosition;
	};
}
