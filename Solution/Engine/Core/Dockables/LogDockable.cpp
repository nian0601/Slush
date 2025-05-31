#include "stdafx.h"
#include "LogDockable.h"

#include "Core/Engine.h"
#include "Core/Log.h"

namespace Slush
{
	void LogDockable::OnBuildUI()
	{
		const FW_GrowingArray<Logger::LogEntry>& entries = Engine::GetInstance().GetLogger().GetEntries();
		ImGui::BeginGroup();
		for (int i = entries.Count() -1; i >= 0; --i)
		{
			const Logger::LogEntry& entry = entries[i];
			Vector3f color = Logger::GetSeverityColorVec(entry.mySeverity);
			ImVec4 imColor;
			imColor.x = color.x;
			imColor.y = color.y;
			imColor.z = color.z;
			imColor.w = 1.f;
			ImGui::TextColored(imColor, entry.myMessage.GetBuffer());
		}

		ImGui::EndGroup();
	}
}