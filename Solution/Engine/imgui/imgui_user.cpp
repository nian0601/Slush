#include "stdafx.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_user.h"
#include <Core\Assets\AssetStorage.h>
#include <Core\Assets\Asset.h>

namespace ImGui
{
	static float s_max_timeline_value;
	static const float TIMELINE_RADIUS = 6;
	static const float TIMELINE_LABEL_MAX_WIDTH = 100.f;

	bool BeginTimeline(const char* str_id, float max_value)
	{
		s_max_timeline_value = max_value;
		return BeginChild(str_id, ImVec2(0, 200));
	}


	bool BeginTimelineTrack(const char* str_id)
	{
		const ImU32 textColor = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Text]);

		ImGui::PushID(str_id);

		ImGuiWindow* win = GetCurrentWindow();
		ImVec2 cursor_pos = win->DC.CursorPos;
		win->DrawList->AddText(cursor_pos, textColor, str_id);
		
		cursor_pos.x += TIMELINE_LABEL_MAX_WIDTH;
		cursor_pos.y += TIMELINE_RADIUS;
		SetCursorScreenPos(cursor_pos);
		return true;
	}


	bool TimelineEvent(const char* str_id, float* values, bool eventIsSelected)
	{
		ImGuiWindow* win = GetCurrentWindow();
		const ImU32 inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
		const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);
		const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonActive]);
		ImVec2 cursor_pos = win->DC.CursorPos;

		ImVec2 leftPointPos;
		ImVec2 rightPointPos;
		for (int i = 0; i < 2; ++i)
		{
			ImVec2 pos = cursor_pos;
			pos.x += win->Size.x * values[i] / s_max_timeline_value + TIMELINE_RADIUS;
			pos.y += TIMELINE_RADIUS;

			ImVec2 cursorScreenPos = pos;
			cursorScreenPos.x -= TIMELINE_RADIUS;
			cursorScreenPos.y -= TIMELINE_RADIUS;
			SetCursorScreenPos(cursorScreenPos);
			PushID(i);
			InvisibleButton(str_id, ImVec2(2 * TIMELINE_RADIUS, 2 * TIMELINE_RADIUS));
			if (IsItemActive() || IsItemHovered())
			{
				ImGui::SetTooltip("%f", values[i]);
				ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y);
				ImVec2 b(pos.x, GetWindowContentRegionMax().y + win->Pos.y);
				win->DrawList->AddLine(a, b, line_color);
			}
			if (IsItemActive() && IsMouseDragging(0))
			{
				values[i] += GetIO().MouseDelta.x / win->Size.x * s_max_timeline_value;
			}
			PopID();
			win->DrawList->AddCircleFilled(
				pos, TIMELINE_RADIUS, IsItemActive() || IsItemHovered() ? active_color : inactive_color);

			if (i == 0)
				leftPointPos = pos;
			else if (i == 1)
				rightPointPos = pos;
		}

		ImVec2 start = cursor_pos;
		start.x += win->Size.x * values[0] / s_max_timeline_value + 2 * TIMELINE_RADIUS;
		start.y += TIMELINE_RADIUS * 0.5f;

		ImVec2 end = start;
		end.x += win->Size.x * (values[1] - values[0]) / s_max_timeline_value - 2 * TIMELINE_RADIUS;
		end.y += TIMELINE_RADIUS;

		//PushID(-1);
		SetCursorScreenPos(start);

		ImVec2 buttonSize = end;
		buttonSize.x -= start.x;
		buttonSize.y -= start.y;
		//InvisibleButton(str_id, buttonSize);
		bool isSelected = Selectable("##SelectableLine", eventIsSelected, 0, buttonSize);

		if (IsItemActive() && IsMouseDragging(0))
		{
			const float mouseMovement = GetIO().MouseDelta.x;

			bool preventDrag = false;
			preventDrag |= mouseMovement < 0 && values[0] <= 0.f;
			preventDrag |= mouseMovement > 0 && values[1] >= s_max_timeline_value;

			if (!preventDrag)
			{
				values[0] += GetIO().MouseDelta.x / win->Size.x * s_max_timeline_value;
				values[1] += GetIO().MouseDelta.x / win->Size.x * s_max_timeline_value;
			}
		}

		//PopID();

		win->DrawList->AddRectFilled(start, end, IsItemActive() || IsItemHovered() || eventIsSelected ? active_color : inactive_color);
		
		SetCursorScreenPos(cursor_pos);

		if (values[0] > values[1])
		{
			float tmp = values[0];
			values[0] = values[1];
			values[1] = tmp;
		}
		if (values[1] > s_max_timeline_value) values[1] = s_max_timeline_value;
		if (values[0] < 0) values[0] = 0;
		return isSelected;
	}


	void EndTimelineTrack()
	{
		ImGui::PopID();

		ImGuiWindow* win = GetCurrentWindow();
		ImVec2 cursor_pos = win->DC.CursorPos;
		cursor_pos.x = win->Pos.x;
		cursor_pos.y += GetTextLineHeightWithSpacing();
		
		SetCursorScreenPos(cursor_pos);
	}

	void EndTimeline()
	{
		ImGuiWindow* win = GetCurrentWindow();

		ImU32 color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
		ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_TextDisabled]);
		ImU32 sub_line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Border]);
		ImU32 text_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Text]);

		ImVec2 start;
		start.x = GetWindowContentRegionMin().x + win->Pos.x;
		//start.x += TIMELINE_LABEL_MAX_WIDTH;
		start.y = GetWindowContentRegionMax().y - GetTextLineHeightWithSpacing() + win->Pos.y;
		
		ImVec2 end = GetWindowContentRegionMax();
		end.x += win->Pos.x;
		end.y += win->Pos.y;

		//float rounding = GImGui->Style.ScrollbarRounding;
		float rounding = 0;
		win->DrawList->AddRectFilled(start, end, color, rounding);

		const int LINE_COUNT = 10;
		const int SUB_LINE_COUNT = 4;
		const float lineSpacing = GetWindowContentRegionWidth() / LINE_COUNT;
		const float subLineSpacing = lineSpacing / (SUB_LINE_COUNT + 1);
		const ImVec2 text_offset(0, GetTextLineHeightWithSpacing());
		for (int i = 0; i < LINE_COUNT+1; ++i)
		{
			ImVec2 a = GetWindowContentRegionMin();
			a.x += win->Pos.x + TIMELINE_RADIUS + TIMELINE_LABEL_MAX_WIDTH;
			a.y += win->Pos.y;

			a.x += i * lineSpacing;
			ImVec2 b = a;
			b.y = start.y;
			win->DrawList->AddLine(a, b, line_color);

			char tmp[256];
			ImFormatString(tmp, sizeof(tmp), "%.2f", i * s_max_timeline_value / LINE_COUNT);
			win->DrawList->AddText(b, text_color, tmp);

			for (int j = 0; j < SUB_LINE_COUNT; ++j)
			{
				a.x += subLineSpacing;
				b = a;
				b.y = start.y;
				win->DrawList->AddLine(a, b, sub_line_color);
			}
		}

		EndChild();
	}

	void BeingDraggedAsset(const Slush::Asset& anAsset, int anAssetIndex)
	{
		ImGuiDragDropFlags src_flags = 0;
		if (ImGui::BeginDragDropSource(src_flags))
		{
			if (!(src_flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
				ImGui::Text("Dragging \"%s\"", anAsset.GetAssetName().GetBuffer());
			ImGui::SetDragDropPayload(anAsset.GetTypeName(), &anAssetIndex, sizeof(int));
			ImGui::EndDragDropSource();
		}
	}

	Slush::Asset* AcceptDraggedAsset(const int aAssetTypeID)
	{
		Slush::IAssetStorage& assetStorage = Slush::AssetRegistry::GetInstance().GetAssetStorage(aAssetTypeID);
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(assetStorage.GetAssetTypeName(), 0))
		{
			int assetIndex = *(const int*)payload->Data;
			return assetStorage.GetAllAssets()[assetIndex];
		}

		return nullptr;
	}

}