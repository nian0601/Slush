#pragma once

namespace Slush
{
	class BaseSprite;

	class Animation;

	struct AnimationRuntime
	{
		enum State
		{
			Waiting,
			Running,
			Finished,
		};

		struct TrackData
		{
			void Reset();

			bool myIsActive = false;
			float myValue = FLT_MAX;
			int myCurrentClip = 0;
		};

		void Restart();

		State myState = Waiting;
		float myElapsedTime = 0.f;

		TrackData myOutlineData;
		TrackData myScaleData;
		TrackData myPositionData;

		Vector2f myStartPosition;
		Vector2f myEndPosition;
		Vector2f myCurrentPosition;
	};

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

		bool Update(float anElapsedTime, AnimationRuntime::TrackData& aTrackData);

	private:
		AnimationClip& AddClip(float aDuration);

		FW_GrowingArray<AnimationClip> myClips;
		float myEndTime = 0.f;
	};

	//////////////////////////////////////////////////////////////////////////
	class Animation
	{
	public:
		Animation(BaseSprite& aSprite);

		void Update(AnimationRuntime& aRuntimeData);

		void MakeOneShot() { myIsLooping = false; }

		AnimationTrack myOutlineTrack;
		AnimationTrack myScaleTrack;
		AnimationTrack myPositionTrack;

	private:
		void ApplyAnimation(AnimationRuntime& aRuntimeData);

		bool myIsLooping = true;

		BaseSprite& mySprite;
	};
}
