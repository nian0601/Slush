#pragma once

namespace Slush
{
	class BaseSprite;

	struct AnimationClip
	{
		enum State
		{
			Waiting,
			Running,
			Finished,
		};

		State myState = Waiting;
		float myStartTime = FLT_MAX;
		float myEndTime = FLT_MAX;

		float myStartValue = 0.f;
		float myEndValue = 0.f;

		void Update(float anElapsedTime, BaseSprite& aSprite);
	};

	class Animation
	{
	public:
		Animation(BaseSprite& aSprite);

		void AddClip(const AnimationClip& aClip);

		void Update();

		void Restart();

		void MakeOneShot() { myIsLooping = false; }

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

		FW_GrowingArray<AnimationClip> myClips;

		BaseSprite& mySprite;
	};
}
