#pragma once
#include <FW_GrowingArray.h>
#include <float.h>
#include "Core\Assets\DataAsset.h"

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

		void OnParse(AssetParser::Handle aHandle);

		enum Type
		{
			None,
			Linear,
			Constant,
		};
		int myType = None;
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

		virtual void OnParse(AssetParser::Handle aHandle);

		Interpolator myInterpolator;

		Vector2f myStartEndTime{ FLT_MAX, FLT_MAX };
		bool myIsSelected = false;
	protected:
		virtual void OnUpdate() {}

		bool myIsWaitingClip = false;
	};

	class SpritesheetClip : public AnimationClip
	{
	public:
		void OnParse(AssetParser::Handle aHandle) override;

		Vector2i myFramePos;
	};

	//////////////////////////////////////////////////////////////////////////

	class AnimationTrack
	{
	public:
		AnimationTrack& Linear(float aDuration, float aStart, float aEnd);
		AnimationTrack& Constant(float aDuration, float aValue);
		AnimationTrack& Wait(float aDuration);

		bool Update(float anElapsedTime, AnimationRuntimeTrackData& aTrackData);
		bool HasClips() const { return !myClips.IsEmpty(); }

		void OnParse(const char* aTrackName, AssetParser::Handle aHandle);
		void BuildUI(const char* aTrackName, AnimationClip*& outSelectedClip);

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

		SpritesheetTrack& Frame(const Vector2i& aFramePosition);

		bool Update(float anElapsedTime, SpritesheetRuntimeTrackData& aTrackData);
		bool HasClips() const { return !myClips.IsEmpty(); }

		void OnParse(const char* aTrackName, AssetParser::Handle aHandle);

	private:
		SpritesheetClip& AddClip(float aDuration);

		FW_GrowingArray<SpritesheetClip> myClips;
		float myEndTime = 0.f;

		float myFPS;
		Vector2i myFrameSize;
	};

	//////////////////////////////////////////////////////////////////////////

	class Animation : public DataAsset
	{
	public:
		DEFINE_ASSET("Animation", "anim", "data/animations");
		Animation(const char* aName);

		void OnParse(AssetParser::Handle aRootHandle);
		void BuildUI();

		void Update(AnimationRuntime& aRuntimeData);

		AnimationTrack myOutlineTrack;
		AnimationTrack myScaleTrack;
		AnimationTrack myPositionTrack;
		AnimationTrack myColorTrack;
		SpritesheetTrack mySpritesheetTrack;

		AnimationClip* mySelectedClip = nullptr;
	};
}
