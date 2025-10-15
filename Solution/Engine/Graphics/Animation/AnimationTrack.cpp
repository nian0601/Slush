#include "stdafx.h"
#include "AnimationTrack.h"
#include "AnimationRuntime.h"
#include "..\Texture.h"

namespace Slush
{
	void AnimationClip::SetStartTimeAndDuration(float aStartTime, float aDuration)
	{
		myStartEndTime.x = aStartTime;
		myStartEndTime.y = myStartEndTime.x + aDuration;
	}

	AnimationClip::State AnimationClip::Update(float anElapsedTime, float& outValue)
	{
		if (anElapsedTime < myStartEndTime.x)
			return State::NotStarted;

		if (anElapsedTime >= myStartEndTime.y)
		{
			outValue = myInterpolator.GetValue(1.f);
			return State::Finished;
		}

		float progress = FW_UnLerp(myStartEndTime.x, myStartEndTime.y, anElapsedTime);
		outValue = myInterpolator.GetValue(progress);

		return State::Running;
	}

	void AnimationClip::OnParse(AssetParser::Handle aHandle)
	{
		aHandle.ParseFloatField("starttime", myStartEndTime.x);
		aHandle.ParseFloatField("endtime", myStartEndTime.y);
		myInterpolator.OnParse(aHandle);

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

	void AnimationClip::BuildUI()
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable;
		if (ImGui::BeginTable("table_scrolly", 2, flags))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None);
			ImGui::TableHeadersRow();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Start Time");

			ImGui::TableSetColumnIndex(1);
			ImGui::DragFloat("##StartTime", &myStartEndTime.x, 0.01f, 0.f, 1.f);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("End Time");

			ImGui::TableSetColumnIndex(1);
			ImGui::DragFloat("##EndTime", &myStartEndTime.y, 0.01f, 0.f, 1.f);


			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Interpolator");

			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(50);
			ImGui::DragFloat("##InterpolatorStart", &myInterpolator.myStartValue, 0.01f, 0.f, 1.f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			ImGui::DragFloat("##InterpolatorEnd", &myInterpolator.myEndValue, 0.01f, 0.f, 1.f);
			ImGui::SameLine();
			const char* interpolatorTypes[] = { "None", "Linear", "Constant" };
			ImGui::Combo("##Interpolator", &myInterpolator.myType, interpolatorTypes, IM_ARRAYSIZE(interpolatorTypes));


			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("FrameRect (X,Y,W,H)");

			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(50);
			ImGui::DragInt("##FrameRectX", &myFrameRect.myTopLeft.x);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			ImGui::DragInt("##FrameRectY", &myFrameRect.myTopLeft.y);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			ImGui::DragInt("##FrameRectW", &myFrameRect.myExtents.x);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			ImGui::DragInt("##FrameRectH", &myFrameRect.myExtents.y);

			myFrameRect = MakeRectFromTopLeft(myFrameRect.myTopLeft, myFrameRect.myExtents);

			ImGui::EndTable();
		}
	}

	//////////////////////////////////////////////////////////////////////////

	AnimationTrack& AnimationTrack::Linear(float aDuration, float aStart, float aEnd)
	{
		AnimationClip& clip = AddClip(aDuration);
		clip.myInterpolator.MakeLinear(aStart, aEnd);

		return *this;
	}

	AnimationTrack& AnimationTrack::Constant(float aDuration, float aValue)
	{
		AnimationClip& clip = AddClip(aDuration);
		clip.myInterpolator.MakeConstant(aValue);

		return *this;
	}

	AnimationTrack& AnimationTrack::Wait(float aDuration)
	{
		float waitValue = 0.f;
		if (myClips.Count() > 0)
			waitValue = myClips.GetLast().myInterpolator.myEndValue;

		return Constant(aDuration, waitValue);
	}

	AnimationTrack& AnimationTrack::Frame(const Vector2i& aFramePosition, const Vector2i& aFrameSize, float aFPS)
	{
		AnimationClip& clip = AddClip(1.f / aFPS);
		clip.myFrameRect = MakeRectFromTopLeft(aFramePosition * aFrameSize, aFrameSize);

		return *this;
	}

	bool AnimationTrack::Update(float anElapsedTime, AnimationRuntimeTrackData& aTrackData)
	{
		if (aTrackData.myCurrentClip >= myClips.Count())
		{
			aTrackData.myIsActive = false;
			return false;
		}

		AnimationClip::State state = myClips[aTrackData.myCurrentClip].Update(anElapsedTime, aTrackData.myValue);
		if (state == AnimationClip::State::Finished)
			++aTrackData.myCurrentClip;

		aTrackData.myIsActive = state != AnimationClip::State::NotStarted;
		return true;
	}

	void AnimationTrack::OnParse(const char* aTrackName, AssetParser::Handle aHandle)
	{
		AssetParser::Handle trackHandle = aHandle.ParseChildElement(aTrackName);

		if (trackHandle.IsReading())
		{
			int numClips = trackHandle.GetNumChildElements();
			myClips.Reserve(numClips);
		}

		for (int i = 0; i < myClips.Count(); ++i)
		{
			AssetParser::Handle clipHandle;
			if (trackHandle.IsReading())
				clipHandle = trackHandle.GetChildElementAtIndex(i);
			else
				clipHandle = trackHandle.ParseChildElement("clip");

			AnimationClip& clip = myClips[i];

			clip.OnParse(clipHandle);
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
					if (ImGui::TimelineEvent("clip", &myClips[i].myStartEndTime.x, &myClips[i] == outSelectedClip))
					{
						outSelectedClip = &myClips[i];
					}
					ImGui::PopID();
				}
			}

			ImGui::EndTimelineTrack();
		}
	}

	AnimationClip& AnimationTrack::AddClip(float aDuration)
	{
		AnimationClip& clip = myClips.Add();
		clip.SetStartTimeAndDuration(myEndTime, aDuration);

		myEndTime += aDuration;

		return clip;
	}

	//////////////////////////////////////////////////////////////////////////

	bool SpritesheetTrack::Update(float anElapsedTime, SpritesheetRuntimeTrackData& aTrackData)
	{
		if (aTrackData.myCurrentClip >= myClips.Count())
		{
			aTrackData.myIsActive = false;
			return false;
		}

		AnimationClip::State state = myClips[aTrackData.myCurrentClip].Update(anElapsedTime, aTrackData.myValue);

		aTrackData.myFrameRect = myClips[aTrackData.myCurrentClip].myFrameRect;

		if (state == AnimationClip::State::Finished)
			++aTrackData.myCurrentClip;

		aTrackData.myIsActive = state != AnimationClip::State::NotStarted;
		return true;
	}
}