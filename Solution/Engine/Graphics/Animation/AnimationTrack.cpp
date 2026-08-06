#include "stdafx.h"
#include "AnimationTrack.h"
#include "AnimationRuntime.h"
#include "..\Texture.h"
#include <imgui\ImGuiWidgets.h>

namespace Slush
{
	void AnimationClip::SetStartTimeAndDuration(float aStartTime, float aDuration)
	{
		myStartEndTime.x = aStartTime;
		myStartEndTime.y = myStartEndTime.x + aDuration;
	}

	void AnimationClip::OnParse(AssetParser::Handle aHandle)
	{
		aHandle.ParseFloatField("starttime", myStartEndTime.x);
		aHandle.ParseFloatField("endtime", myStartEndTime.y);
	}

	AnimationClip::State AnimationClip::GetTimeState(float anElapsedTime, float& outProgress) const
	{
		if (anElapsedTime < myStartEndTime.x)
			return State::NotStarted;

		if (anElapsedTime >= myStartEndTime.y)
		{
			outProgress = 1.f;
			return State::Finished;
		}

		outProgress = FW_UnLerp(myStartEndTime.x, myStartEndTime.y, anElapsedTime);
		return State::Running;
	}

	void AnimationClip::BuildTimeRowsUI()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Start Time");

		ImGui::TableSetColumnIndex(1);
		Slush::ImGuiWidgets::DragFloat("##StartTime", &myStartEndTime.x, 0.01f, 0.f, 1.f);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("End Time");

		ImGui::TableSetColumnIndex(1);
		Slush::ImGuiWidgets::DragFloat("##EndTime", &myStartEndTime.y, 0.01f, 0.f, 1.f);
	}

	AnimationClip* AnimationClip::CreateClip(Type aType)
	{
		if (aType == Type::SpriteSheet)
			return new SpriteSheetClip();

		return new FloatClip(aType);
	}

	//////////////////////////////////////////////////////////////////////////

	AnimationClip::State FloatClip::Update(float anElapsedTime, float& outValue) const
	{
		float progress = 0.f;
		State state = GetTimeState(anElapsedTime, progress);
		outValue = myInterpolator.GetValue(progress);
		return state;
	}

	void FloatClip::OnParse(AssetParser::Handle aHandle)
	{
		AnimationClip::OnParse(aHandle);
		myInterpolator.OnParse(aHandle);
	}

	void FloatClip::BuildUI()
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable;
		if (ImGui::BeginTable("table_scrolly", 2, flags))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None);
			ImGui::TableHeadersRow();

			BuildTimeRowsUI();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Interpolator");

			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(50);
			Slush::ImGuiWidgets::DragFloat("##InterpolatorStart", &myInterpolator.myStartValue, 0.01f, 0.f, 1.f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			Slush::ImGuiWidgets::DragFloat("##InterpolatorEnd", &myInterpolator.myEndValue, 0.01f, 0.f, 1.f);
			ImGui::SameLine();
			const char* interpolatorTypes[] = { "None", "Linear", "Constant" };
			Slush::ImGuiWidgets::Combo("##Interpolator", &myInterpolator.myType, interpolatorTypes, IM_ARRAYSIZE(interpolatorTypes));

			ImGui::EndTable();
		}
	}

	//////////////////////////////////////////////////////////////////////////

	AnimationClip::State SpriteSheetClip::Update(float anElapsedTime, float& outValue) const
	{
		float progress = 0.f;
		State state = GetTimeState(anElapsedTime, progress);
		outValue = 0.f;
		return state;
	}

	void SpriteSheetClip::OnParse(AssetParser::Handle aHandle)
	{
		AnimationClip::OnParse(aHandle);

		AssetParser::Handle frameRectHandle;
		if (aHandle.IsReading())
			frameRectHandle = aHandle.ParseChildElement("frameRect");
		else if (myFrameRect.myExtents.x > 0.f)
			frameRectHandle = aHandle.ParseChildElement("frameRect");

		if (frameRectHandle.IsValid())
		{
			frameRectHandle.ParseIntField("x", myFrameRect.myTopLeft.x);
			frameRectHandle.ParseIntField("y", myFrameRect.myTopLeft.y);
			frameRectHandle.ParseIntField("width", myFrameRect.myExtents.x);
			frameRectHandle.ParseIntField("height", myFrameRect.myExtents.y);
		}
	}

	void SpriteSheetClip::BuildUI()
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable;
		if (ImGui::BeginTable("table_scrolly", 2, flags))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None);
			ImGui::TableHeadersRow();

			BuildTimeRowsUI();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("FrameRect (X,Y,W,H)");

			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(50);
			Slush::ImGuiWidgets::DragInt("##FrameRectX", &myFrameRect.myTopLeft.x);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			Slush::ImGuiWidgets::DragInt("##FrameRectY", &myFrameRect.myTopLeft.y);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			Slush::ImGuiWidgets::DragInt("##FrameRectW", &myFrameRect.myExtents.x);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			Slush::ImGuiWidgets::DragInt("##FrameRectH", &myFrameRect.myExtents.y);

			myFrameRect = MakeRectFromTopLeft(myFrameRect.myTopLeft, myFrameRect.myExtents);

			ImGui::EndTable();
		}
	}

	//////////////////////////////////////////////////////////////////////////

	AnimationTrack::~AnimationTrack()
	{
		RemoveAllClips();
	}

	AnimationTrack& AnimationTrack::Linear(AnimationClip::Type aType, float aDuration, float aStart, float aEnd)
	{
		AnimationClip& clip = AddClip(aType, aDuration);
		static_cast<FloatClip&>(clip).myInterpolator.MakeLinear(aStart, aEnd);

		return *this;
	}

	AnimationTrack& AnimationTrack::Constant(AnimationClip::Type aType, float aDuration, float aValue)
	{
		AnimationClip& clip = AddClip(aType, aDuration);
		static_cast<FloatClip&>(clip).myInterpolator.MakeConstant(aValue);

		return *this;
	}

	AnimationTrack& AnimationTrack::Wait(AnimationClip::Type aType, float aDuration)
	{
		float waitValue = 0.f;
		if (myClips.Count() > 0 && myClips.GetLast()->GetType() != AnimationClip::Type::SpriteSheet)
			waitValue = static_cast<FloatClip*>(myClips.GetLast())->myInterpolator.myEndValue;

		return Constant(aType, aDuration, waitValue);
	}

	AnimationTrack& AnimationTrack::Frame(AnimationClip::Type aType, const Vector2i& aFramePosition, const Vector2i& aFrameSize, float aFPS)
	{
		AnimationClip& clip = AddClip(aType, 1.f / aFPS);
		static_cast<SpriteSheetClip&>(clip).myFrameRect = MakeRectFromTopLeft(aFramePosition * aFrameSize, aFrameSize);

		return *this;
	}

	bool AnimationTrack::Update(float anElapsedTime, AnimationRuntimeTrackData& aTrackData) const
	{
		if (aTrackData.myCurrentClip >= myClips.Count())
		{
			aTrackData.myIsActive = false;
			return false;
		}

		AnimationClip* clip = myClips[aTrackData.myCurrentClip];
		AnimationClip::State state = clip->Update(anElapsedTime, aTrackData.myValue);

		aTrackData.myActiveClipType = clip->GetType();
		if (clip->GetType() == AnimationClip::Type::SpriteSheet)
			aTrackData.myFrameRect = static_cast<SpriteSheetClip*>(clip)->myFrameRect;

		if (state == AnimationClip::State::Finished)
			++aTrackData.myCurrentClip;

		aTrackData.myIsActive = state != AnimationClip::State::NotStarted;
		return true;
	}

	void AnimationTrack::RemoveAllClips()
	{
		myClips.DeleteAll();
		myEndTime = 0.f;
	}

	const AnimationClip* AnimationTrack::GetFirstClipOfType(AnimationClip::Type aType) const
	{
		for (AnimationClip* clip : myClips)
		{
			if (clip->GetType() == aType)
				return clip;
		}

		return nullptr;
	}

	bool AnimationTrack::HasClipOfType(AnimationClip::Type aType) const
	{
		return GetFirstClipOfType(aType) != nullptr;
	}

	void AnimationTrack::OnParse(AssetParser::Handle aTrackHandle)
	{
		if (aTrackHandle.IsReading())
		{
			int numClips = aTrackHandle.GetNumChildElements();
			myClips.Reserve(numClips);
			for (int i = 0; i < numClips; ++i)
			{
				AssetParser::Handle clipHandle = aTrackHandle.GetChildElementAtIndex(i);

				int typeAsInt = 0;
				clipHandle.ParseIntField("type", typeAsInt);

				myClips[i] = AnimationClip::CreateClip(static_cast<AnimationClip::Type>(typeAsInt));
				myClips[i]->OnParse(clipHandle);
			}
		}
		else
		{
			for (int i = 0; i < myClips.Count(); ++i)
			{
				AssetParser::Handle clipHandle = aTrackHandle.ParseChildElement("clip");

				int typeAsInt = static_cast<int>(myClips[i]->GetType());
				clipHandle.ParseIntField("type", typeAsInt);

				myClips[i]->OnParse(clipHandle);
			}
		}
	}

	void AnimationTrack::OnParseLegacy(const char* aTrackName, AnimationClip::Type aType, AssetParser::Handle aRootHandle)
	{
		AssetParser::Handle trackHandle = aRootHandle.ParseChildElement(aTrackName);
		if (!trackHandle.IsValid())
			return;

		int numClips = trackHandle.GetNumChildElements();
		myClips.Reserve(numClips);
		for (int i = 0; i < numClips; ++i)
		{
			myClips[i] = AnimationClip::CreateClip(aType);
			myClips[i]->OnParse(trackHandle.GetChildElementAtIndex(i));
		}
	}

	void AnimationTrack::BuildUI(const char* aTrackName, AnimationClip*& outSelectedClip)
	{
		if (ImGui::BeginTimelineTrack(aTrackName))
		{
			if (myClips.Count() > 0)
			{
				for (int i = 0; i < myClips.Count(); ++i)
				{
					ImGui::PushID(i);
					if (ImGui::TimelineEvent("clip", &myClips[i]->myStartEndTime.x, myClips[i] == outSelectedClip))
					{
						outSelectedClip = myClips[i];
					}
					ImGui::PopID();
				}
			}

			ImGui::EndTimelineTrack();
		}
	}

	AnimationClip& AnimationTrack::AddClip(AnimationClip::Type aType, float aDuration)
	{
		AnimationClip* clip = AnimationClip::CreateClip(aType);
		clip->SetStartTimeAndDuration(myEndTime, aDuration);

		for (AnimationClip* existingClip : myClips)
		{
			bool overlaps = clip->myStartEndTime.x < existingClip->myStartEndTime.y && existingClip->myStartEndTime.x < clip->myStartEndTime.y;
			FW_ASSERT(!overlaps, "AnimationTrack: new clip overlaps with an existing clip in this track");
		}

		myEndTime += aDuration;
		myClips.Add(clip);

		return *clip;
	}
}
