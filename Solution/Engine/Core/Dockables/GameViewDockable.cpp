#include "stdafx.h"
#include "GameViewDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"

namespace Slush
{
	void GameViewDockable::OnBuildUI()
	{
		Engine& engine = Engine::GetInstance();
		engine.GetWindow().RenderOffscreenBufferToImGUI();

		engine.myByPassImGUIInputRestriction = ImGui::IsItemHovered();

		OnBuildOverlay(engine.GetWindow().GetGameViewRect());
	}

	void GameViewDockable::DrawOverlayText(const Vector2f& aScreenPosition, const char* aText, unsigned int aColor)
	{
		const ImVec2 padding = ImGui::GetStyle().FramePadding;
		const ImVec2 textSize = ImGui::CalcTextSize(aText);
		const ImVec2 textPos(aScreenPosition.x + padding.x, aScreenPosition.y + padding.y);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(ImVec2(aScreenPosition.x, aScreenPosition.y),
			ImVec2(textPos.x + textSize.x + padding.x, textPos.y + textSize.y + padding.y),
			ImGui::GetColorU32(ImGuiCol_WindowBg, 0.85f), ImGui::GetStyle().FrameRounding);
		drawList->AddText(textPos, aColor, aText);
	}
}