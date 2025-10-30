#pragma once
#include <FW_GrowingArray.h>
#include <float.h>
#include "Core\Assets\DataAsset.h"

#include "AnimationTrack.h"

namespace Slush
{
	struct AnimationRuntime;
	class Texture;

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

	private:
		void HandleSpritesheetImport();
		void HandleTextureInteraction();

		struct ToolData
		{
			bool myWantToImportTexture = false;
			const Texture* myTextureToImport = nullptr;
			Vector2i myFrameSize = { 48, 48 };
			Vector2i myFrameCount = { 8, 8 };
			bool myUseFrameSize = true;
			bool myShowFullTexture = false;
			Vector2i myStartFrameIndex = { -1, -1 };
			Vector2i myEndFrameIndex = { -1, -1 };
		};
		ToolData myToolData;
		
	};
}
