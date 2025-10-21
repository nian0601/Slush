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
		static Vector2i startFrameIndex = { -1, -1 };
		static Vector2i endFrameIndex = { -1, -1 };
		if (myWantToImportTexture)
		{
			ImGui::OpenPopup("Import_Texture");
			modalOpenstate = true;
			myWantToImportTexture = false;

			startFrameIndex = { -1, -1 };
			endFrameIndex = { -1, -1 };
		}

		if (myTextureToImport)
		{
			ImGui::SetNextWindowSize({800, 600 });
			if (ImGui::BeginPopupModal("Import_Texture", &modalOpenstate))
			{
				const Vector2i& textureSize = myTextureToImport->GetSize();
				ImGui::BeginChild("texture_helper", { 0, -200 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
				
				static Vector2f frameSize = { 48.f, 48.f };
				static Vector2i frameCount = { 8, 8 };
				static bool useFrameSize = true;
				static bool showFullTexture = false;

				if (showFullTexture)
				{
					ImVec2 pos = ImGui::GetCursorScreenPos(); // Needs to be BEFORE we render the image, else the cursor-position is wrong
					ImGuiIO& io = ImGui::GetIO();
					const float localX = io.MousePos.x - pos.x;
					const float localY = io.MousePos.y - pos.y;

					const float textureWidth = static_cast<float>(textureSize.x);
					const float textureHeight = static_cast<float>(textureSize.y);
					ImGui::Image(*myTextureToImport->GetSFMLTexture(), { textureWidth, textureHeight });

					if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
					{
						ImGui::Text("(%i, %i)", static_cast<int>(localX), static_cast<int>(localY));
						ImGui::EndTooltip();
					}
				}
				else
				{
					int xCount = frameCount.x;
					int yCount = frameCount.y;
					if (useFrameSize)
					{
						xCount = static_cast<int>(textureSize.x / frameSize.x);
						yCount = static_cast<int>(textureSize.y / frameSize.y);
					}
					else
					{
						frameCount.x = FW_Max(1, frameCount.x);
						frameCount.y = FW_Max(1, frameCount.y);
						frameSize.x = static_cast<float>(textureSize.x / frameCount.x);
						frameSize.y = static_cast<float>(textureSize.y / frameCount.y);
					}

					for (int y = 0; y < yCount; ++y)
					{
						ImGui::PushID(y);
						for (int x = 0; x < xCount; ++x)
						{
							ImGui::PushID(x);
							const float xPos = x * frameSize.x;
							const float yPos = y * frameSize.y;
							sf::FloatRect rect = { {xPos, yPos}, {frameSize.x, frameSize.y} };
							sf::Color color = sf::Color::Transparent;

							if (x == startFrameIndex.x && y == startFrameIndex.y)
								color = sf::Color::Yellow;
							if (x == endFrameIndex.x && y == endFrameIndex.y)
								color = sf::Color::Yellow;

							if (ImGui::ImageButton("frameButton", *myTextureToImport->GetSFMLTexture(), { frameSize.x, frameSize.y }, rect, color))
							{
								if (startFrameIndex.x == -1)
								{
									startFrameIndex.x = x;
									startFrameIndex.y = y;
								}
								else
								{
									endFrameIndex.x = x;
									endFrameIndex.y = y;
								}
							}

							ImGui::PopID();
							ImGui::SameLine();
						}
						ImGui::PopID();
						ImGui::NewLine();
					}
				}

				ImGui::EndChild();

				ImGui::Checkbox("Show Full texture", &showFullTexture);
				ImGui::Checkbox("Use FrameSize", &useFrameSize);
				if (useFrameSize)
				{
					ImGui::DragFloat("X Size", &frameSize.x);
					ImGui::DragFloat("Y Size", &frameSize.y);
				}
				else
				{
					ImGui::DragInt("X Count", &frameCount.x);
					ImGui::DragInt("Y Count", &frameCount.y);
				}

				ImGui::EndPopup();
			}
		}
	}

}