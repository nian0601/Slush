#pragma once
#include <FW_String.h>

namespace Slush
{
	class Asset;
}

namespace ImGui
{
	// ImGui::InputText() with FW_String
	// Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
	IMGUI_API bool InputText(const char* label, FW_String* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	IMGUI_API bool InputTextMultiline(const char* label, FW_String* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	IMGUI_API bool InputTextWithHint(const char* label, const char* hint, FW_String* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);

	IMGUI_API bool BeginTimeline(const char* str_id, float max_value);
	IMGUI_API bool BeginTimelineTrack(const char* str_id);
	IMGUI_API bool TimelineEvent(const char* str_id, float* values, bool eventIsSelected);
	IMGUI_API void EndTimelineTrack();
	IMGUI_API void EndTimeline();


	IMGUI_API void BeingDraggedAsset(const Slush::Asset& anAsset, int anAssetIndex);
	IMGUI_API Slush::Asset* AcceptDraggedAsset(const int aAssetTypeID);
}
