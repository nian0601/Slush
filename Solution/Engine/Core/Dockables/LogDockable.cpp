#include "stdafx.h"
#include "LogDockable.h"

#include "Core/Engine.h"
#include "Core/Log.h"

namespace Slush
{
	void LogDockable::OnBuildUI()
	{
		ImGui::Checkbox("Info", &myShowInfo);
		ImGui::SameLine();
		ImGui::Checkbox("Debug", &myShowDebug);
		ImGui::SameLine();
		ImGui::Checkbox("Warning", &myShowWarning);
		ImGui::SameLine();
		ImGui::Checkbox("Error", &myShowError);
		ImGui::SameLine();
		ImGui::Text("|");
		ImGui::SameLine();
		ImGui::Checkbox("Track Latest", &myTrackLatestMessage);

		const FW_GrowingArray<Logger::LogEntry>& entries = Engine::GetInstance().GetLogger().GetEntries();
		static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV ;
		if (ImGui::BeginTable("logdockable_table", 2, flags))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 60);
			ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_None);
			ImGui::TableHeadersRow();

			for (int i = 0; i < entries.Count(); ++i)
			{
				const Logger::LogEntry& entry = entries[i];
				if (entry.mySeverity == Logger::INFO && !myShowInfo)
					continue;

				if (entry.mySeverity == Logger::DEBUG && !myShowDebug)
					continue;

				if (entry.mySeverity == Logger::WARNING && !myShowWarning)
					continue;

				if (entry.mySeverity == Logger::ERROR && !myShowError)
					continue;

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::Text(Logger::GetSeverityText(entry.mySeverity));

				ImGui::TableSetColumnIndex(1);

				Vector3f color = Logger::GetSeverityColorVec(entry.mySeverity);
				ImVec4 imColor;
				imColor.x = color.x;
				imColor.y = color.y;
				imColor.z = color.z;
				imColor.w = 1.f;
				ImGui::TextColored(imColor, entry.myMessage.GetBuffer());
			}

			if (myTrackLatestMessage)
				ImGui::SetScrollY(ImGui::GetScrollMaxY());

			ImGui::EndTable();
		}
	}
}