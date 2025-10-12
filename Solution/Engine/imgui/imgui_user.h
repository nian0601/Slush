#pragma once

namespace ImGui
{
	IMGUI_API bool BeginTimeline(const char* str_id, float max_value);
	IMGUI_API bool BeginTimelineTrack(const char* str_id);
	IMGUI_API bool TimelineEvent(const char* str_id, float* values, bool eventIsSelected);
	IMGUI_API void EndTimelineTrack();
	IMGUI_API void EndTimeline();
}
