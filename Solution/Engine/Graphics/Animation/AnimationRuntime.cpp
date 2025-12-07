#include "stdafx.h"

#include "Graphics/Animation/Animation.h"
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

	void AnimationRuntime::Start(BaseSprite& aSprite, const Animation& anAnimation)
	{
		if (myState == Running)
			Stop(aSprite, anAnimation);	

		myState = Running;
		myElapsedTime = 0.f;

		myOutlineData.Start(aSprite);
		myOutlineData.myValue = 1.f;
		myScaleData.Start(aSprite);
		myScaleData.myValue = 1.f;
		myPositionData.Start(aSprite);
		myColorData.Start(aSprite);
		mySpritesheetData.Start(aSprite);

		if (anAnimation.myTexture)
			aSprite.SetTexture(*anAnimation.myTexture);
	}

	void AnimationRuntime::Stop(BaseSprite& aSprite, const Animation& /*anAnimation*/)
	{
		mySpritesheetData.End(aSprite);
	}
}