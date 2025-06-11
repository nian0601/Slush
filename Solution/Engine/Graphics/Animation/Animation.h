#pragma once
#include <FW_GrowingArray.h>
#include <float.h>

namespace Slush
{
	class Animation;
	class BaseSprite;
	class Texture;

	struct AnimationRuntime;
	struct AnimationRuntimeTrackData;
	struct SpritesheetRuntimeTrackData;


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
		virtual void OnUpdate() {}

		float myStartTime = FLT_MAX;
		float myEndTime = FLT_MAX;
		bool myIsWaitingClip = false;
	};

	class SpritesheetClip : public AnimationClip
	{
	public:
		Vector2i myFramePos;
		Texture* myTexture = nullptr;
	};

	//////////////////////////////////////////////////////////////////////////

	class AnimationTrack
	{
	public:
		AnimationTrack& Linear(float aDuration, float aStart, float aEnd);
		AnimationTrack& Constant(float aDuration, float aValue);
		AnimationTrack& Wait(float aDuration);

		bool Update(float anElapsedTime, AnimationRuntimeTrackData& aTrackData);

	protected:
		AnimationClip& AddClip(float aDuration);

		FW_GrowingArray<AnimationClip> myClips;
		float myEndTime = 0.f;
	};

	class SpritesheetTrack
	{
	public:
		void SetFPS(float aFPS) { myFPS = aFPS; }
		void SetFrameSize(const Vector2i& aFrameSize) { myFrameSize = aFrameSize; }

		SpritesheetTrack& Frame(const Vector2i& aFramePosition, Texture* aTexture = nullptr);

		bool Update(float anElapsedTime, SpritesheetRuntimeTrackData& aTrackData);

	private:
		SpritesheetClip& AddClip(float aDuration);

		FW_GrowingArray<SpritesheetClip> myClips;
		float myEndTime = 0.f;

		float myFPS;
		Vector2i myFrameSize;
	};

	//////////////////////////////////////////////////////////////////////////

	class Animation
	{
	public:
		void Update(AnimationRuntime& aRuntimeData);

		void MakeOneShot() { myIsLooping = false; }

		AnimationTrack myOutlineTrack;
		AnimationTrack myScaleTrack;
		AnimationTrack myPositionTrack;
		AnimationTrack myColorTrack;
		SpritesheetTrack mySpritesheetTrack;

	private:
		void ApplyAnimation(AnimationRuntime& aRuntimeData);

		bool myIsLooping = true;
	};
}
