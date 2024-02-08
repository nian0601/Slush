#pragma once
#include <FW_GrowingArray.h>
#include <float.h>

namespace Slush
{
	class Animation;
	class BaseSprite;

	struct AnimationRuntime;
	struct AnimationRuntimeTrackData;

	//////////////////////////////////////////////////////////////////////////

	struct Interpolator
	{
		float GetValue(float aProgress);

		void MakeLinear(float aStart, float aEnd);
		void MakeConstant(float aValue);

	private:
		enum Type
		{
			None,
			Linear,
			Constant,
		};
		Type myType = None;
		float myStartValue = 0.f;
		float myEndValue = 0.f;
	};

	//////////////////////////////////////////////////////////////////////////

	class AnimationClip
	{
	public: 
		enum State
		{
			NotStarted,
			Running,
			Finished,
		};

		void SetStartTimeAndDuration(float aStartTime, float aDuration);
		void MakeWaitingClip() { myIsWaitingClip = true; }

		State Update(float anElapsedTime, float& outValue);

		Interpolator myInterpolator;

	protected:
		float myStartTime = FLT_MAX;
		float myEndTime = FLT_MAX;
		bool myIsWaitingClip = false;
	};

	//////////////////////////////////////////////////////////////////////////

	class AnimationTrack
	{
	public:
		AnimationTrack& Linear(float aDuration, float aStart, float aEnd);
		AnimationTrack& Constant(float aDuration, float aValue);
		AnimationTrack& Wait(float aDuration);

		bool Update(float anElapsedTime, AnimationRuntimeTrackData& aTrackData);

	private:
		AnimationClip& AddClip(float aDuration);

		FW_GrowingArray<AnimationClip> myClips;
		float myEndTime = 0.f;
	};

	//////////////////////////////////////////////////////////////////////////

	class Animation
	{
	public:
		void Update(AnimationRuntime& aRuntimeData, BaseSprite& aSprite);

		void MakeOneShot() { myIsLooping = false; }

		AnimationTrack myOutlineTrack;
		AnimationTrack myScaleTrack;
		AnimationTrack myPositionTrack;

	private:
		void ApplyAnimation(AnimationRuntime& aRuntimeData, BaseSprite& aSprite);

		bool myIsLooping = true;
	};
}
