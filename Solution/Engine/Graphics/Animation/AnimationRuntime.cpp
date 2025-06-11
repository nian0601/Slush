#include "stdafx.h"

#include "Graphics/Animation/AnimationRuntime.h"
#include "Graphics/BaseSprite.h"

namespace Slush
{
	void AnimationRuntimeTrackData::Start(BaseSprite& /*aSprite*/)
	{
		myIsActive = false;
		myValue = 0.f;
		myCurrentClip = 0;
	}

	void SpritesheetRuntimeTrackData::Start(BaseSprite& aSprite)
	{
		AnimationRuntimeTrackData::Start(aSprite);

		myPreviousTexture = aSprite.GetTexture();
		myPreviousFrameRect = aSprite.GetTextureRect();
		myFrameRect = MakeRect(0, 0, 0, 0);
	}

	void SpritesheetRuntimeTrackData::End(BaseSprite& aSprite)
	{
		AnimationRuntimeTrackData::End(aSprite);

		if (myPreviousTexture)
			aSprite.SetTexture(*myPreviousTexture);

		if (myPreviousFrameRect.myExtents.x > 0.f)
			aSprite.SetTextureRect(myPreviousFrameRect.myTopLeft.x, myPreviousFrameRect.myTopLeft.y, myPreviousFrameRect.myExtents.x, myPreviousFrameRect.myExtents.y);
	}

	//////////////////////////////////////////////////////////////////////////

	AnimationRuntime::AnimationRuntime(BaseSprite& aSprite)
		: mySprite(aSprite)
	{
	}

	void AnimationRuntime::Start()
	{
		if (myState == Running)
			Stop();	

		myState = Running;
		myElapsedTime = 0.f;

		myOutlineData.Start(mySprite);
		myOutlineData.myValue = 1.f;
		myScaleData.Start(mySprite);
		myScaleData.myValue = 1.f;
		myPositionData.Start(mySprite);
		myColorData.Start(mySprite);
		mySpritesheetData.Start(mySprite);
	}

	void AnimationRuntime::Stop()
	{
		mySpritesheetData.End(mySprite);
	}
}