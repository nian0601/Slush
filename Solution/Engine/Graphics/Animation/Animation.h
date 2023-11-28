#pragma once

namespace Slush
{
	class BaseSprite;

	class Animation;

	struct Interpolator
	{
		float GetValue(float aProgress);

		void MakeLinear(float aStart, float aEnd);

	private:
		enum Type
		{
			None,
			Linear,
		};
		Type myType = None;
		float myStartValue = 0.f;
		float myEndValue = 0.f;
	};

	//////////////////////////////////////////////////////////////////////////

	class AnimationClip
	{
		friend class AnimationTrack;
	public: 

		void SetStartTimeAndDuration(float aStartTime, float aDuration);

		float Update(float anElapsedTime);

		Interpolator myInterpolator;

	protected:
		enum State
		{
			Waiting,
			Running,
			Finished,
		};

		State myState = Waiting;
		float myStartTime = FLT_MAX;
		float myEndTime = FLT_MAX;
		float myValue = 0.f;
	};

	//////////////////////////////////////////////////////////////////////////

	class Animation;
	class AnimationTrack
	{
	public:
		enum TrackType
		{
			Outline,
			Scale,
		};

		AnimationTrack(Animation& anParentAnimation, TrackType aTrackType);

		AnimationTrack& Linear(float aDuration, float aStart, float aEnd);
		AnimationTrack& Wait(float aDuration);

		void Update(float anElapsedTime, BaseSprite& aSprite);
		void Reset();

	private:
		void ApplyAnimation(float aValue, BaseSprite& aSprite);

		Animation& myParentAnimation;
		TrackType myTrackType;
		FW_GrowingArray<AnimationClip> myClips;
		float myEndTime = 0.f;
	};

	//////////////////////////////////////////////////////////////////////////
	class Animation
	{
		friend class AnimationTrack;
	public:
		Animation(BaseSprite& aSprite);

		void Update();
		void Restart();

		void MakeOneShot() { myIsLooping = false; }

		AnimationTrack myOutlineTrack;
		AnimationTrack myScaleTrack;

	private:
		enum State
		{
			Waiting,
			Running,
			Finished,
		};

		State myState = Waiting;

		float myElapsedTime = 0.f;
		float myTotalTime = 0.f;
		bool myIsLooping = true;

		BaseSprite& mySprite;
	};
}
