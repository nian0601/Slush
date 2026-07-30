#pragma once
#include <FW_GrowingArray.h>
#include <FW_Vector2.h>
#include <float.h>

#include "AnimationTrack.h"

namespace Slush
{
	class Animation;
	class BaseSprite;
	class Texture;

	struct AnimationRuntimeTrackData
	{
		void Start(BaseSprite& aSprite);
		void End(BaseSprite& aSprite);

		bool myIsActive = false;
		float myValue = FLT_MAX;
		int myCurrentClip = 0;
		ClipType myActiveClipType = ClipType::Outline;

		Recti myFrameRect;
		const Texture* myPreviousTexture = nullptr;
		Recti myPreviousFrameRect;
	};

	struct AnimationRuntime
	{
		enum State
		{
			NotStarted,
			Running,
			Finished,
		};

		void Start(BaseSprite& aSprite, const Animation& anAnimation);
		void Stop(BaseSprite& aSprite, const Animation& anAnimation);

		bool IsFinished() const { return myState == Finished; }

		State myState = NotStarted;
		bool myIsLooping = false;

		float myElapsedTime = 0.f;

		FW_GrowingArray<AnimationRuntimeTrackData> myTrackData;

		Vector2f myStartPosition;
		Vector2f myEndPosition;
		Vector2f myCurrentPosition;

		int myStartColor;
		int myEndColor;
		int myCurrentColor;
	};
}
