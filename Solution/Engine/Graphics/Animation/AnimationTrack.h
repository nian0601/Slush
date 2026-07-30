#pragma once

#include "Interpolator.h"

namespace Slush
{
	struct AnimationRuntimeTrackData;

	class AnimationClip
	{
	public:
		enum State
		{
			NotStarted,
			Running,
			Finished,
		};

		enum Type
		{
			Outline,
			Scale,
			Position,
			Color,
			SpriteSheet,
		};

		static AnimationClip* CreateClip(Type aType);

		AnimationClip(Type aType) : myType(aType) {}
		virtual ~AnimationClip() {}

		Type GetType() const { return myType; }

		void SetStartTimeAndDuration(float aStartTime, float aDuration);

		virtual State Update(float anElapsedTime, float& outValue) const = 0;

		virtual void OnParse(AssetParser::Handle aHandle);
		virtual void BuildUI() = 0;

		Vector2f myStartEndTime{ FLT_MAX, FLT_MAX };
		bool myIsSelected = false;

	protected:
		State GetTimeState(float anElapsedTime, float& outProgress) const;
		void BuildTimeRowsUI();

	private:
		Type myType;
	};

	class FloatClip : public AnimationClip
	{
	public:
		FloatClip(Type aType) : AnimationClip(aType) {}

		State Update(float anElapsedTime, float& outValue) const override;

		void OnParse(AssetParser::Handle aHandle) override;
		void BuildUI() override;

		Interpolator myInterpolator;
	};

	class SpriteSheetClip : public AnimationClip
	{
	public:
		SpriteSheetClip() : AnimationClip(Type::SpriteSheet) {}

		State Update(float anElapsedTime, float& outValue) const override;

		void OnParse(AssetParser::Handle aHandle) override;
		void BuildUI() override;

		Recti myFrameRect;
	};

	// Owns its clips (deleted via RemoveAllClips) - stored via Animation::myTracks as
	// owned pointers, so it's never copied; delete copy/assign to make that explicit.
	class AnimationTrack
	{
	public:
		AnimationTrack() = default;
		AnimationTrack(const AnimationTrack&) = delete;
		AnimationTrack& operator=(const AnimationTrack&) = delete;
		~AnimationTrack();

		AnimationTrack& Linear(AnimationClip::Type aType, float aDuration, float aStart, float aEnd);
		AnimationTrack& Constant(AnimationClip::Type aType, float aDuration, float aValue);
		AnimationTrack& Wait(AnimationClip::Type aType, float aDuration);
		AnimationTrack& Frame(AnimationClip::Type aType, const Vector2i& aFramePosition, const Vector2i& aFrameSize, float aFPS);

		bool Update(float anElapsedTime, AnimationRuntimeTrackData& aTrackData) const;
		bool HasClips() const { return !myClips.IsEmpty(); }
		void RemoveAllClips();

		const AnimationClip* GetFirstClipOfType(AnimationClip::Type aType) const;
		bool HasClipOfType(AnimationClip::Type aType) const;

		void OnParse(AssetParser::Handle aTrackHandle);
		void OnParseLegacy(const char* aTrackName, AnimationClip::Type aType, AssetParser::Handle aRootHandle);
		void BuildUI(const char* aTrackName, AnimationClip*& outSelectedClip);

	protected:
		AnimationClip& AddClip(AnimationClip::Type aType, float aDuration);

		FW_GrowingArray<AnimationClip*> myClips;
		float myEndTime = 0.f;
	};
}
