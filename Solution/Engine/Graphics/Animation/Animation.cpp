#include "stdafx.h"

#include "Graphics/Animation/Animation.h"
#include "Graphics/Animation/AnimationRuntime.h"
#include "Graphics/BaseSprite.h"
#include "Core/Time.h"

namespace Slush
{
	void Animation::OnParse(AssetParser::Handle aRootHandle)
	{
		myOutlineTrack.OnParse("outlinetrack", aRootHandle);
		myScaleTrack.OnParse("scaletrack", aRootHandle);
		myPositionTrack.OnParse("postiontrack", aRootHandle);
		myColorTrack.OnParse("colortrack", aRootHandle);
		mySpritesheetTrack.OnParse("spritesheettrack", aRootHandle);
	}

	void Animation::BuildUI()
	{
		if (ImGui::BeginTimeline("AnimTimeline", 1.f))
		{
			myOutlineTrack.BuildUI("Outline", mySelectedClip);
			myScaleTrack.BuildUI("Scale", mySelectedClip);
			myPositionTrack.BuildUI("Position", mySelectedClip);
			myColorTrack.BuildUI("Color", mySelectedClip);
			mySpritesheetTrack.BuildUI("SpriteSheet", mySelectedClip);
			ImGui::EndTimeline();
		}

		if (mySelectedClip)
		{
			mySelectedClip->BuildUI();
		}
	}

	void Animation::Update(AnimationRuntime& aRuntimeData)
	{
		if (aRuntimeData.myState != AnimationClip::Running)
			return;

		aRuntimeData.myElapsedTime += Time::GetDelta();

		bool anyTrackActive = false;
		anyTrackActive |= myOutlineTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myOutlineData);
		anyTrackActive |= myScaleTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myScaleData);
		anyTrackActive |= myPositionTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myPositionData);
		anyTrackActive |= myColorTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.myColorData);
		anyTrackActive |= mySpritesheetTrack.Update(aRuntimeData.myElapsedTime, aRuntimeData.mySpritesheetData);

		if (!anyTrackActive)
			aRuntimeData.myState = AnimationRuntime::Finished;
	}
}