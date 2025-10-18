#include "stdafx.h"

#include "Graphics/Animation/Animation.h"
#include "Graphics/Animation/AnimationRuntime.h"
#include "Graphics/BaseSprite.h"
#include "Core/Time.h"
#include "../Texture.h"

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

			if (ImGui::BeginDragDropTarget())
			{
				if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<Texture>()))
				{
					myWantToImportTexture = true;
					myTextureToImport = static_cast<const Texture*>(asset);
				}

				ImGui::EndDragDropTarget();
			}
			ImGui::EndTimeline();
		}

		

		HandleTextureImport();

		if (mySelectedClip)
		{
			mySelectedClip->BuildUI();
		}
	}

	void Animation::Update(AnimationRuntime& aRuntimeData)
	{
		if (aRuntimeData.myState != AnimationRuntime::Running)
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

	void Animation::HandleTextureImport()
	{
		static bool modalOpenstate = true;
		if (myWantToImportTexture)
		{
			ImGui::OpenPopup("Import_Texture");
			modalOpenstate = true;
			myWantToImportTexture = false;
		}

		if (myTextureToImport)
		{
			ImGui::SetNextWindowSize({800, 600 });
			if (ImGui::BeginPopupModal("Import_Texture", &modalOpenstate))
			{
				const float textureWidth = static_cast<float>(myTextureToImport->GetSize().x);
				const float textureHeight = static_cast<float>(myTextureToImport->GetSize().y);
				//ImGui::SetNextWindowSize({ FW_Min(textureWidth, 500.f), FW_Min(textureHeight, 300.f) + 60 });
				ImGui::BeginChild("texture_helper", { 0, -200 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
				ImGui::Image(*myTextureToImport->GetSFMLTexture(), { textureWidth, textureHeight });
				ImGui::EndChild();

				ImGui::EndPopup();
			}
		}
	}

}