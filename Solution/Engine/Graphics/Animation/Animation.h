#pragma once
#include <FW_GrowingArray.h>
#include <float.h>
#include "Core\Assets\DataAsset.h"
#include "Core/Assets/AssetReference.h"

#include "AnimationTrack.h"

namespace Slush
{
	struct AnimationRuntime;
	class RectSprite;
	class Texture;

	class Animation : public DataAsset
	{
	public:
		DEFINE_ASSET("Animation", "anim", "animations", ICON_FA_FILM, 2);
		Animation(const char* aName, unsigned int aAssetID);
		~Animation();

		void OnParse(AssetParser::Handle aRootHandle, unsigned int aVersion) override;
		void ResolveDependencies() override;
		void BuildUI();

		void Update(AnimationRuntime& aRuntimeData) const;

		bool HasSpriteSheetClip() const;
		const AnimationClip* FindFirstSpriteSheetClip() const;

		FW_GrowingArray<AnimationTrack*> myTracks;
		AssetReference<Texture> myTexture;

	private:
		void ParseLegacyTracks(AssetParser::Handle aRootHandle);
		AnimationTrack& FindOrCreateSpritesheetTrack();

		void HandleSpritesheetImport();
		void HandleTextureInteraction();
		void HandlePreview();

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
			int myFPS = 15;
			AnimationClip* mySelectedClip = nullptr;

			AnimationRuntime* myRuntime = nullptr;
			RectSprite* myPreviewSprite = nullptr;
			float myPreviewScale = 1.f;
		};
		ToolData myToolData;
		
	};
}
