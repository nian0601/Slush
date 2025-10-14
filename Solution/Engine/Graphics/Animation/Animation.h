#pragma once
#include <FW_GrowingArray.h>
#include <float.h>
#include "Core\Assets\DataAsset.h"

#include "AnimationTrack.h"

namespace Slush
{
	struct AnimationRuntime;

	class Animation : public DataAsset
	{
	public:
		DEFINE_ASSET("Animation", "anim", "data/animations");
		using DataAsset::DataAsset;

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
