#include "stdafx.h"

#include "Graphics/Animation/Animation.h"
#include "Graphics/Animation/AnimationRuntime.h"
#include "Graphics/BaseSprite.h"
#include "Graphics/RectSprite.h"
#include "Graphics/Texture.h"
#include "Core/Assets/AssetStorage.h"
#include "Core/Time.h"

namespace Slush
{

	Animation::Animation(const char* aName, unsigned int aAssetID)
		: DataAsset(aName, aAssetID)
	{
		myTracks.Add(new AnimationTrack());

		myToolData.myRuntime = new AnimationRuntime();
		myToolData.myPreviewSprite = new RectSprite();
		myToolData.myPreviewSprite->SetSize(98.f, 98.f);
	}

	Animation::~Animation()
	{
		myTracks.DeleteAll();

		FW_SAFE_DELETE(myToolData.myPreviewSprite);
		FW_SAFE_DELETE(myToolData.myRuntime);
	}

	void Animation::OnParse(AssetParser::Handle aRootHandle, unsigned int aVersion)
	{
		if (aRootHandle.IsReading())
		{
			myTracks.DeleteAll();

			if (aVersion < 2)
			{
				ParseLegacyTracks(aRootHandle);
			}
			else
			{
				AssetParser::Handle tracksHandle = aRootHandle.ParseChildElement("tracks");
				int numTracks = tracksHandle.GetNumChildElements();
				for (int i = 0; i < numTracks; ++i)
				{
					AnimationTrack* track = new AnimationTrack();
					track->OnParse(tracksHandle.GetChildElementAtIndex(i));
					myTracks.Add(track);
				}
			}

			if (aRootHandle.HasField("spritesheettexture"))
			{
				FW_String texID;
				aRootHandle.ParseStringField("spritesheettexture", texID);
				Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();

				// This texture should be saved in something thats not meant to be tool-specific.
				// And update how animations are updated, to make it apply this texture when this animation is started etc
				myTexture = assets.GetAsset<Slush::Texture>(texID.GetBuffer());
				FW_ASSERT(myTexture != nullptr, "Invalid texture used for spritesheet-animation");

				if (const AnimationClip* clip = FindFirstSpriteSheetClip())
				{
					Recti frameRect = static_cast<const SpriteSheetClip*>(clip)->myFrameRect;

					myToolData.myFrameSize = frameRect.myExtents;
					myToolData.myPreviewSprite->SetSize(static_cast<float>(myToolData.myFrameSize.x), static_cast<float>(myToolData.myFrameSize.y));
				}
			}
		}
		else
		{
			AssetParser::Handle tracksHandle = aRootHandle.ParseChildElement("tracks");
			for (int i = 0; i < myTracks.Count(); ++i)
			{
				if (!myTracks[i]->HasClips())
					continue;

				myTracks[i]->OnParse(tracksHandle.ParseChildElement("track"));
			}

			if (myTexture)
			{
				FW_String texID = myTexture->GetAssetName();
				aRootHandle.ParseStringField("spritesheettexture", texID);
			}
		}
	}

	void Animation::ParseLegacyTracks(AssetParser::Handle aRootHandle)
	{
		struct LegacyTrackInfo
		{
			const char* myName;
			AnimationClip::Type myType;
		};

		static const LegacyTrackInfo legacyTracks[] =
		{
			{ "outlinetrack", AnimationClip::Type::Outline },
			{ "scaletrack", AnimationClip::Type::Scale },
			{ "postiontrack", AnimationClip::Type::Position },
			{ "colortrack", AnimationClip::Type::Color },
			{ "spritesheettrack", AnimationClip::Type::SpriteSheet },
		};

		for (const LegacyTrackInfo& info : legacyTracks)
		{
			AnimationTrack* legacyTrack = new AnimationTrack();
			legacyTrack->OnParseLegacy(info.myName, info.myType, aRootHandle);

			if (legacyTrack->HasClips())
				myTracks.Add(legacyTrack);
			else
				delete legacyTrack;
		}
	}

	void Animation::BuildUI()
	{
		HandlePreview();

		FW_String timelineLable = "AnimTimeLine##";
		timelineLable += GetAssetName();
		if (ImGui::BeginTimeline(timelineLable.GetBuffer(), 1.f))
		{
			for (int i = 0; i < myTracks.Count(); ++i)
			{
				FW_String trackLabel = "Track ";
				trackLabel += i;
				myTracks[i]->BuildUI(trackLabel.GetBuffer(), myToolData.mySelectedClip);
			}

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
		for (int i = 0; i < myTracks.Count() && i < aRuntimeData.myTrackData.Count(); ++i)
			anyTrackActive |= myTracks[i]->Update(aRuntimeData.myElapsedTime, aRuntimeData.myTrackData[i]);

		if (!anyTrackActive)
			aRuntimeData.myState = AnimationRuntime::Finished;
	}

	bool Animation::HasSpriteSheetClip() const
	{
		return FindFirstSpriteSheetClip() != nullptr;
	}

	const AnimationClip* Animation::FindFirstSpriteSheetClip() const
	{
		for (const AnimationTrack* track : myTracks)
		{
			if (const AnimationClip* clip = track->GetFirstClipOfType(AnimationClip::Type::SpriteSheet))
				return clip;
		}

		return nullptr;
	}

	AnimationTrack& Animation::FindOrCreateSpritesheetTrack()
	{
		for (AnimationTrack* track : myTracks)
		{
			if (track->HasClipOfType(AnimationClip::Type::SpriteSheet))
				return *track;
		}

		for (AnimationTrack* track : myTracks)
		{
			if (!track->HasClips())
				return *track;
		}

		AnimationTrack* newTrack = new AnimationTrack();
		myTracks.Add(newTrack);
		return *newTrack;
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
					AnimationTrack& spritesheetTrack = FindOrCreateSpritesheetTrack();
					spritesheetTrack.RemoveAllClips();

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
							spritesheetTrack.Frame(AnimationClip::Type::SpriteSheet, { x, y }, myToolData.myFrameSize, static_cast<float>(myToolData.myFPS));
					}

					myToolData.myPreviewSprite->SetSize(static_cast<float>(myToolData.myFrameSize.x), static_cast<float>(myToolData.myFrameSize.y));
					myTexture = myToolData.myTextureToImport;
					ImGui::CloseCurrentPopup();
				}

				if (cantImport)
					ImGui::SetItemTooltip("Select Start and EndFrame to import");

				ImGui::EndDisabled();

				ImGui::SetNextItemWidth(150.f);
				ImGui::DragInt("FPS", &myToolData.myFPS, 1, 1);

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

	void Animation::HandlePreview()
	{
		if (!HasSpriteSheetClip())
			return;

		FW_ASSERT(myTexture);

		ImGui::BeginDisabled(myToolData.myRuntime->myState != AnimationRuntime::NotStarted);
		if (ImGui::Button("Start"))
			myToolData.myRuntime->Start(*myToolData.myPreviewSprite, *this);
		ImGui::EndDisabled();

		ImGui::SameLine();

		ImGui::BeginDisabled(myToolData.myRuntime->myState == AnimationRuntime::NotStarted);
		if (ImGui::Button("Stop"))
			myToolData.myRuntime->Stop(*myToolData.myPreviewSprite, *this);
		ImGui::EndDisabled();

		ImGui::SameLine();

		ImGui::Checkbox("Loop", &myToolData.myRuntime->myIsLooping);

		ImGui::SameLine();

		ImGui::SetNextItemWidth(150.f);
		ImGui::DragFloat("Preview Scale", &myToolData.myPreviewScale, 1.f, 0.1f, 10.f, "%.1f");


		Recti frameRect = static_cast<const SpriteSheetClip*>(FindFirstSpriteSheetClip())->myFrameRect;
		if (myToolData.myRuntime->myState != AnimationRuntime::NotStarted)
		{
			for (const AnimationRuntimeTrackData& trackData : myToolData.myRuntime->myTrackData)
			{
				if (trackData.myIsActive && trackData.myActiveClipType == AnimationClip::Type::SpriteSheet)
				{
					frameRect = trackData.myFrameRect;
					break;
				}
			}

			if (myToolData.myRuntime->IsFinished() && myToolData.myRuntime->myIsLooping)
			{
				myToolData.myRuntime->Start(*myToolData.myPreviewSprite, *this);
			}
		}

		Update(*myToolData.myRuntime);

		const float width = static_cast<float>(myToolData.myPreviewSprite->GetSize().x);
		const float height = static_cast<float>(myToolData.myPreviewSprite->GetSize().y);

		sf::FloatRect rect;
		rect.position.x = static_cast<float>(frameRect.myTopLeft.x);
		rect.position.y = static_cast<float>(frameRect.myTopLeft.y);
		rect.size.x = static_cast<float>(frameRect.myExtents.x);
		rect.size.y = static_cast<float>(frameRect.myExtents.y);
		ImGui::Image(*myTexture->GetSFMLTexture(), { width * myToolData.myPreviewScale, height * myToolData.myPreviewScale }, rect);
	}

}