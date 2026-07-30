#include "stdafx.h"

#include "Graphics/Animation/Animation.h"
#include "Graphics/Animation/AnimationRuntime.h"
#include "Graphics/BaseSprite.h"

namespace Slush
{
	void AnimationRuntimeTrackData::Start(BaseSprite& aSprite)
	{
		myIsActive = false;
		myValue = 0.f;
		myCurrentClip = 0;

		myPreviousTexture = aSprite.GetTexture();
		myPreviousFrameRect = aSprite.GetTextureRect();
		myFrameRect = MakeRect(0, 0, 0, 0);
	}

	void AnimationRuntimeTrackData::End(BaseSprite& aSprite)
	{
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

		myTrackData.RemoveAll();
		for (int i = 0; i < anAnimation.myTracks.Count(); ++i)
		{
			AnimationRuntimeTrackData& trackData = myTrackData.Add();
			trackData.Start(aSprite);

			if (anAnimation.myTracks[i]->HasClipOfType(AnimationClip::Type::Outline) || anAnimation.myTracks[i]->HasClipOfType(AnimationClip::Type::Scale))
				trackData.myValue = 1.f;
		}

		if (anAnimation.myTexture)
			aSprite.SetTexture(*anAnimation.myTexture);
	}

	void AnimationRuntime::Stop(BaseSprite& aSprite, const Animation& /*anAnimation*/)
	{
		myState = NotStarted;

		for (int i = 0; i < myTrackData.Count(); ++i)
			myTrackData[i].End(aSprite);
	}
}
