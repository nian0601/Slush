#pragma once
#include <FW_Vector2.h>
#include <float.h>

namespace Slush
{
	class BaseSprite;
	class Texture;

	struct AnimationRuntimeTrackData
	{
		virtual void Start(BaseSprite& aSprite);
		virtual void End(BaseSprite& aSprite) { aSprite; };

		bool myIsActive = false;
		float myValue = FLT_MAX;
		int myCurrentClip = 0;
	};

	struct SpritesheetRuntimeTrackData : public AnimationRuntimeTrackData
	{
		void Start(BaseSprite& aSprite) override;
		void End(BaseSprite& aSprite) override;

		Recti myFrameRect;

		const Texture* myPreviousTexture;
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

		AnimationRuntime(BaseSprite& aSprite);

		void Start();
		void Stop();

		bool IsFinished() const { return myState == Finished; }

		BaseSprite& mySprite;
		State myState = NotStarted;
		float myElapsedTime = 0.f;

		AnimationRuntimeTrackData myOutlineData;
		AnimationRuntimeTrackData myScaleData;
		AnimationRuntimeTrackData myPositionData;
		AnimationRuntimeTrackData myColorData;
		SpritesheetRuntimeTrackData mySpritesheetData;

		Vector2f myStartPosition;
		Vector2f myEndPosition;
		Vector2f myCurrentPosition;

		int myStartColor;
		int myEndColor;
		int myCurrentColor;
	};
}
