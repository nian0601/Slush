#include "stdafx.h"

#include "Graphics/Animation/Animation.h"
#include "Graphics/Animation/AnimationRuntime.h"
#include "Graphics/BaseSprite.h"
#include "Graphics/Texture.h"
#include "Core/Assets/AssetStorage.h"
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

		if (aRootHandle.IsReading())
		{
			if (aRootHandle.HasField("spritesheettexture"))
			{
				FW_String texID;
				aRootHandle.ParseStringField("spritesheettexture", texID);
				Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();

				// This texture should be saved in something thats not meant to be tool-specific.
				// And update how animations are updated, to make it apply this texture when this animation is started etc
				myTexture = assets.GetAsset<Slush::Texture>(texID.GetBuffer());
				FW_ASSERT(myTexture != nullptr, "Invalid texture used for spritesheet-animation");
			}
		}
		else
		{
			if (myToolData.myTextureToImport)
			{
				FW_String texID = myToolData.myTextureToImport->GetAssetName();
				aRootHandle.ParseStringField("spritesheettexture", texID);
			}
		}
	}

	void Animation::BuildUI()
	{
		if (ImGui::BeginTimeline("AnimTimeline", 1.f))
		{
			myOutlineTrack.BuildUI("Outline", myToolData.mySelectedClip);
			myScaleTrack.BuildUI("Scale", myToolData.mySelectedClip);
			myPositionTrack.BuildUI("Position", myToolData.mySelectedClip);
			myColorTrack.BuildUI("Color", myToolData.mySelectedClip);
			mySpritesheetTrack.BuildUI("SpriteSheet", myToolData.mySelectedClip);

			if (ImGui::BeginDragDropTarget())
			{
				if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<Texture>()))
				{
					myToolData.myWantToImportTexture = true;
					myToolData.myTextureToImport = static_cast<const Texture*>(asset);
				}

				ImGui::EndDragDropTarget();
			}
			ImGui::EndTimeline();
		}

		HandleSpritesheetImport();

		if (myToolData.mySelectedClip)
		{
			myToolData.mySelectedClip->BuildUI();
		}
	}

	void Animation::Update(AnimationRuntime& aRuntimeData) const
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

	void Animation::HandleSpritesheetImport()
	{
		static bool modalOpenstate = true;
		if (myToolData.myWantToImportTexture)
		{
			ImGui::OpenPopup("Import_Texture");
			modalOpenstate = true;
			myToolData.myWantToImportTexture = false;

			myToolData.myStartFrameIndex = { -1, -1 };
			myToolData.myEndFrameIndex = { -1, -1 };
		}

		if (myToolData.myTextureToImport)
		{
			ImGui::SetNextWindowSize({ 800, 600 });
			if (ImGui::BeginPopupModal("Import_Texture", &modalOpenstate))
			{
				ImGui::BeginChild("texture_helper", { 0, -200 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);

				HandleTextureInteraction();

				ImGui::EndChild();

				const bool cantImport  = myToolData.myEndFrameIndex.x == -1;
				ImGui::BeginDisabled(cantImport);
				if (ImGui::Button("Import"))
				{
					mySpritesheetTrack.RemoveAllClips();

					for (int y = myToolData.myStartFrameIndex.y; y <= myToolData.myEndFrameIndex.y; ++y)
					{
						Vector2i startEndX;
						if (y == myToolData.myStartFrameIndex.y)
							startEndX.x = myToolData.myStartFrameIndex.x;
						else if (y > myToolData.myStartFrameIndex.y)
							startEndX.x = 0;

						if (y == myToolData.myEndFrameIndex.y)
							startEndX.y = myToolData.myEndFrameIndex.x;
						else if (y < myToolData.myEndFrameIndex.y)
							startEndX.y = myToolData.myFrameCount.x;

						for (int x = startEndX.x; x < startEndX.y; ++x)
							mySpritesheetTrack.Frame({ x, y }, myToolData.myFrameSize, 15.f);
					}

					myTexture = myToolData.myTextureToImport;
					ImGui::CloseCurrentPopup();
				}

				if (cantImport)
					ImGui::SetItemTooltip("Select Start and EndFrame to import");

				ImGui::EndDisabled();

				ImGui::Separator();

				ImGui::Checkbox("Show Full texture", &myToolData.myShowFullTexture);
				ImGui::Checkbox("Use FrameSize", &myToolData.myUseFrameSize);
				if (myToolData.myUseFrameSize)
				{
					ImGui::DragInt("X Size", &myToolData.myFrameSize.x);
					ImGui::DragInt("Y Size", &myToolData.myFrameSize.y);
				}
				else
				{
					ImGui::DragInt("X Count", &myToolData.myFrameCount.x);
					ImGui::DragInt("Y Count", &myToolData.myFrameCount.y);
				}

				ImGui::EndPopup();
			}
		}
	}

	void Animation::HandleTextureInteraction()
	{
		const Vector2i& textureSize = myToolData.myTextureToImport->GetSize();

		if (myToolData.myShowFullTexture)
		{
			ImVec2 pos = ImGui::GetCursorScreenPos(); // Needs to be BEFORE we render the image, else the cursor-position is wrong
			ImGuiIO& io = ImGui::GetIO();
			const float localX = io.MousePos.x - pos.x;
			const float localY = io.MousePos.y - pos.y;

			const float textureWidth = static_cast<float>(textureSize.x);
			const float textureHeight = static_cast<float>(textureSize.y);
			ImGui::Image(*myToolData.myTextureToImport->GetSFMLTexture(), { textureWidth, textureHeight });

			if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
			{
				ImGui::Text("(%i, %i)", static_cast<int>(localX), static_cast<int>(localY));
				ImGui::EndTooltip();
			}
		}
		else
		{
			if (myToolData.myUseFrameSize)
			{
				myToolData.myFrameCount.x = textureSize.x / myToolData.myFrameSize.x;
				myToolData.myFrameCount.y = textureSize.y / myToolData.myFrameSize.y;
			}
			else
			{
				myToolData.myFrameCount.x = FW_Max(1, myToolData.myFrameCount.x);
				myToolData.myFrameCount.y = FW_Max(1, myToolData.myFrameCount.y);
				myToolData.myFrameSize.x = textureSize.x / myToolData.myFrameCount.x;
				myToolData.myFrameSize.y = textureSize.y / myToolData.myFrameCount.y;
			}

			for (int y = 0; y < myToolData.myFrameCount.y; ++y)
			{
				ImGui::PushID(y);

				Vector2i minMaxHighlightX = { INT_MAX, -1 };
				if (y == myToolData.myStartFrameIndex.y)
					minMaxHighlightX.x = myToolData.myStartFrameIndex.x;
				else if (y > myToolData.myStartFrameIndex.y)
					minMaxHighlightX.x = 0;

				if (y == myToolData.myEndFrameIndex.y)
					minMaxHighlightX.y = myToolData.myEndFrameIndex.x;
				else if (y < myToolData.myEndFrameIndex.y)
					minMaxHighlightX.y = myToolData.myFrameCount.x;

				for (int x = 0; x < myToolData.myFrameCount.x; ++x)
				{
					ImGui::PushID(x);
					const float width = static_cast<float>(myToolData.myFrameSize.x);
					const float height = static_cast<float>(myToolData.myFrameSize.y);

					const float xPos = x * width;
					const float yPos = y * height;
					sf::FloatRect rect = { {xPos, yPos}, {width, height} };
					sf::Color color = sf::Color::Transparent;

					bool useHighlight = false;
					if (x >= minMaxHighlightX.x && x <= minMaxHighlightX.y)
						useHighlight = true;

					if (useHighlight)
						color = sf::Color::Yellow;

					if (ImGui::ImageButton("frameButton", *myToolData.myTextureToImport->GetSFMLTexture(), { width, height }, rect, color))
					{
						if (myToolData.myStartFrameIndex.x == -1)
						{
							myToolData.myStartFrameIndex.x = x;
							myToolData.myStartFrameIndex.y = y;
							myToolData.myEndFrameIndex = myToolData.myStartFrameIndex;
						}
						else
						{
							myToolData.myEndFrameIndex.x = x;
							myToolData.myEndFrameIndex.y = y;
						}
					}

					ImGui::PopID();
					ImGui::SameLine();
				}
				ImGui::PopID();
				ImGui::NewLine();
			}
		}
	}
}