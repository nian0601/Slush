#pragma once

#include "Interpolator.h"

namespace Slush
{
	struct AnimationRuntimeTrackData;
	struct SpritesheetRuntimeTrackData;

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

		State Update(float anElapsedTime, float& outValue) const;

		virtual void OnParse(AssetParser::Handle aHandle);
		void BuildUI();

		Interpolator myInterpolator;
		Recti myFrameRect;

		Vector2f myStartEndTime{ FLT_MAX, FLT_MAX };
		bool myIsSelected = false;
	};

	class AnimationTrack
	{
	public:
		AnimationTrack& Linear(float aDuration, float aStart, float aEnd);
		AnimationTrack& Constant(float aDuration, float aValue);
		AnimationTrack& Wait(float aDuration);
		AnimationTrack& Frame(const Vector2i& aFramePosition, const Vector2i& aFrameSize, float aFPS);

		bool Update(float anElapsedTime, AnimationRuntimeTrackData& aTrackData) const;
		bool HasClips() const { return !myClips.IsEmpty(); }
		void RemoveAllClips() { myClips.RemoveAll(); }

		void OnParse(const char* aTrackName, AssetParser::Handle aHandle);
		void BuildUI(const char* aTrackName, AnimationClip*& outSelectedClip);

	protected:
		AnimationClip& AddClip(float aDuration);

		FW_GrowingArray<AnimationClip> myClips;
		float myEndTime = 0.f;
	};

	class SpritesheetTrack : public AnimationTrack
	{
	public:
		bool Update(float anElapsedTime, SpritesheetRuntimeTrackData& aTrackData) const;
	};
}
