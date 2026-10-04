#include "stdafx.h"

#include "Core/EditorTheme.h"

#include <FW_FileSystem.h>

#include "imgui/Fonts/IconsFontAwesome7.h"

namespace Slush
{
	namespace EditorTheme
	{
		// Colors are named by role, not value, so a palette is a values-only change.
		// Values come from Glöd's design system (ClaudeTools/docs/design-system.md)
		// and the dashboard-redesign-e prototype.
		struct Colors
		{
			ImVec4 myGround;		// Deepest background: title bars, menu bar, unselected tabs
			ImVec4 myPanel;			// Window background
			ImVec4 myRaised;		// Frames, buttons, popups
			ImVec4 myLine;			// Hairlines: borders, separators
			ImVec4 myHover;
			ImVec4 myPress;
			ImVec4 mySelect;		// Selected rows (Header)
			ImVec4 mySelectHover;
			ImVec4 mySelectActive;
			ImVec4 myInk;			// Body text
			ImVec4 myDim;			// Disabled / secondary text
			ImVec4 myScrollGrab;
			ImVec4 myScrollGrabHover;
			ImVec4 myAccent;		// Tab overline, slider grab, docking preview
			ImVec4 myAccentBright;	// Check marks, nav cursor, links
			ImVec4 myAttention;		// What is burning: drag-drop target, unsaved marker
			ImVec4 myPlot;
			ImVec4 myPlotLines;
		};

		static ImVec4 Hex(unsigned int aRgb, float anAlpha = 1.f)
		{
			const float r = static_cast<float>((aRgb >> 16) & 0xFF) / 255.f;
			const float g = static_cast<float>((aRgb >> 8) & 0xFF) / 255.f;
			const float b = static_cast<float>(aRgb & 0xFF) / 255.f;
			return ImVec4(r, g, b, anAlpha);
		}

		static ImVec4 WithAlpha(const ImVec4& aColor, float anAlpha)
		{
			return ImVec4(aColor.x, aColor.y, aColor.z, anAlpha);
		}

		static void FillNeutrals(Colors& aColors)
		{
			aColors.myGround = Hex(0x0a0c0f);
			aColors.myPanel = Hex(0x0f1216);
			aColors.myRaised = Hex(0x121419);
			aColors.myLine = Hex(0x262a31);
			aColors.myHover = Hex(0x2a2f37);
			aColors.myPress = Hex(0x2f3a44);
			aColors.myInk = Hex(0xd6d8dc);
			aColors.myDim = Hex(0x7e858f);
			aColors.myScrollGrab = Hex(0x262a31);
			aColors.myScrollGrabHover = Hex(0x5f656e);
		}

		static Colors GetColdColors()
		{
			Colors colors;
			FillNeutrals(colors);

			// Slush cold spectrum (design-system §3.3)
			colors.mySelect = Hex(0x1e252c);
			colors.mySelectHover = Hex(0x2f3a44);
			colors.mySelectActive = Hex(0x4a5a68);
			colors.myAccent = Hex(0x6f8fa6);
			colors.myAccentBright = Hex(0x9db8cc);

			colors.myAttention = Hex(0xef8744);
			colors.myPlot = Hex(0xe8c17a);
			colors.myPlotLines = Hex(0x9db8cc);
			return colors;
		}

		static Colors GetWarmColors()
		{
			Colors colors;
			FillNeutrals(colors);

			// Glöd warm spectrum (design-system §3.2, prototype --warm-* tokens)
			colors.mySelect = Hex(0x1d1410);
			colors.mySelectHover = Hex(0x4a2e1f);
			colors.mySelectActive = Hex(0x6d4a37);
			colors.myAccent = Hex(0xef8744);
			colors.myAccentBright = Hex(0xf0ad85);

			// Orange is already the accent here, so attention moves to ash gold
			colors.myAttention = Hex(0xe8c17a);
			colors.myPlot = Hex(0xe8c17a);
			colors.myPlotLines = Hex(0xf0ad85);
			return colors;
		}

		static void ApplyShape(ImGuiStyle& aStyle)
		{
			// Dense, instrument-panel spacing
			aStyle.WindowPadding = ImVec2(10.f, 10.f);
			aStyle.FramePadding = ImVec2(8.f, 5.f);
			aStyle.ItemSpacing = ImVec2(8.f, 6.f);
			aStyle.ItemInnerSpacing = ImVec2(6.f, 4.f);
			aStyle.CellPadding = ImVec2(6.f, 4.f);
			aStyle.IndentSpacing = 18.f;
			aStyle.ScrollbarSize = 12.f;
			aStyle.GrabMinSize = 10.f;

			// Hairlines, not boxes
			aStyle.WindowBorderSize = 1.f;
			aStyle.ChildBorderSize = 1.f;
			aStyle.PopupBorderSize = 1.f;
			aStyle.FrameBorderSize = 1.f;
			aStyle.TabBorderSize = 0.f;
			aStyle.TabBarBorderSize = 1.f;
			aStyle.TabBarOverlineSize = 2.f;

			// --radius 6px for frames/buttons, --radius-lg 8px for popups; docked windows and tabs stay square
			aStyle.WindowRounding = 0.f;
			aStyle.ChildRounding = 8.f;
			aStyle.FrameRounding = 6.f;
			aStyle.PopupRounding = 8.f;
			aStyle.ScrollbarRounding = 6.f;
			aStyle.GrabRounding = 6.f;
			aStyle.TabRounding = 0.f;

			aStyle.WindowTitleAlign = ImVec2(0.f, 0.5f);
		}

		static void ApplyColors(ImGuiStyle& aStyle, const Colors& aColors)
		{
			ImVec4* c = aStyle.Colors;
			const ImVec4 transparent(0.f, 0.f, 0.f, 0.f);

			c[ImGuiCol_Text] = aColors.myInk;
			c[ImGuiCol_TextDisabled] = aColors.myDim;
			c[ImGuiCol_WindowBg] = aColors.myPanel;
			c[ImGuiCol_ChildBg] = transparent;
			c[ImGuiCol_PopupBg] = aColors.myRaised;
			c[ImGuiCol_Border] = aColors.myLine;
			c[ImGuiCol_BorderShadow] = transparent;

			c[ImGuiCol_FrameBg] = aColors.myRaised;
			c[ImGuiCol_FrameBgHovered] = aColors.myHover;
			c[ImGuiCol_FrameBgActive] = aColors.myPress;

			c[ImGuiCol_TitleBg] = aColors.myGround;
			c[ImGuiCol_TitleBgActive] = aColors.myPanel;
			c[ImGuiCol_TitleBgCollapsed] = aColors.myGround;
			c[ImGuiCol_MenuBarBg] = aColors.myGround;

			c[ImGuiCol_ScrollbarBg] = transparent;
			c[ImGuiCol_ScrollbarGrab] = aColors.myScrollGrab;
			c[ImGuiCol_ScrollbarGrabHovered] = aColors.myScrollGrabHover;
			c[ImGuiCol_ScrollbarGrabActive] = aColors.myAccent;

			c[ImGuiCol_CheckMark] = aColors.myAccentBright;
			c[ImGuiCol_SliderGrab] = aColors.myAccent;
			c[ImGuiCol_SliderGrabActive] = aColors.myAccentBright;

			c[ImGuiCol_Button] = aColors.myRaised;
			c[ImGuiCol_ButtonHovered] = aColors.myHover;
			c[ImGuiCol_ButtonActive] = aColors.myPress;

			c[ImGuiCol_Header] = aColors.mySelect;
			c[ImGuiCol_HeaderHovered] = aColors.mySelectHover;
			c[ImGuiCol_HeaderActive] = aColors.mySelectActive;

			c[ImGuiCol_Separator] = aColors.myLine;
			c[ImGuiCol_SeparatorHovered] = aColors.myAccent;
			c[ImGuiCol_SeparatorActive] = aColors.myAccentBright;

			c[ImGuiCol_ResizeGrip] = transparent;
			c[ImGuiCol_ResizeGripHovered] = WithAlpha(aColors.myAccent, 0.6f);
			c[ImGuiCol_ResizeGripActive] = aColors.myAccentBright;

			c[ImGuiCol_InputTextCursor] = aColors.myAccentBright;

			c[ImGuiCol_TabHovered] = aColors.mySelectHover;
			c[ImGuiCol_Tab] = aColors.myGround;
			c[ImGuiCol_TabSelected] = aColors.myPanel;
			c[ImGuiCol_TabSelectedOverline] = aColors.myAccent;
			c[ImGuiCol_TabDimmed] = aColors.myGround;
			c[ImGuiCol_TabDimmedSelected] = aColors.myPanel;
			c[ImGuiCol_TabDimmedSelectedOverline] = transparent;

			c[ImGuiCol_DockingPreview] = WithAlpha(aColors.myAccent, 0.4f);
			c[ImGuiCol_DockingEmptyBg] = aColors.myGround;

			c[ImGuiCol_PlotLines] = aColors.myPlotLines;
			c[ImGuiCol_PlotLinesHovered] = aColors.myAttention;
			c[ImGuiCol_PlotHistogram] = aColors.myPlot;
			c[ImGuiCol_PlotHistogramHovered] = aColors.myAttention;

			c[ImGuiCol_TableHeaderBg] = aColors.myGround;
			c[ImGuiCol_TableBorderStrong] = aColors.myLine;
			c[ImGuiCol_TableBorderLight] = aColors.myLine;
			c[ImGuiCol_TableRowBg] = transparent;
			c[ImGuiCol_TableRowBgAlt] = WithAlpha(aColors.myRaised, 0.6f);

			c[ImGuiCol_TextLink] = aColors.myAccentBright;
			c[ImGuiCol_TextSelectedBg] = WithAlpha(aColors.myAccent, 0.35f);
			c[ImGuiCol_TreeLines] = aColors.myLine;
			c[ImGuiCol_DragDropTarget] = aColors.myAttention;
			c[ImGuiCol_UnsavedMarker] = aColors.myAttention;
			c[ImGuiCol_NavCursor] = aColors.myAccentBright;
			c[ImGuiCol_NavWindowingHighlight] = WithAlpha(aColors.myAccentBright, 0.7f);
			c[ImGuiCol_NavWindowingDimBg] = WithAlpha(aColors.myGround, 0.6f);
			c[ImGuiCol_ModalWindowDimBg] = WithAlpha(aColors.myGround, 0.6f);
		}

		void ApplyPalette(Palette aPalette)
		{
			ImGuiStyle& style = ImGui::GetStyle();

			// Start from a clean default so switching palettes never leaves stale values
			style = ImGuiStyle();
			ImGui::StyleColorsDark(&style);

			switch (aPalette)
			{
			case IMGUI_DEFAULT:
				return;
			case COLD:
				ApplyShape(style);
				ApplyColors(style, GetColdColors());
				return;
			case WARM:
				ApplyShape(style);
				ApplyColors(style, GetWarmColors());
				return;
			}

			FW_ASSERT(false, "Unknown EditorTheme::Palette");
		}

		void LoadFonts(Font aFont)
		{
			FW_String path;
			float size = 20.f;
			switch (aFont)
			{
			case NOTO_SANS:
				path = "Data/NotoSans.ttf";
				size = 20.f;
				break;
			case JETBRAINS_MONO:
				// Monospace runs wider than NotoSans, so a slightly smaller size keeps panels comparable
				path = "Data/JetBrainsMono-Regular.ttf";
				size = 18.f;
				break;
			}
			FW_FileSystem::GetAbsoluteFilePath(path, path);

			ImGuiIO& imguiIO = ImGui::GetIO();
			imguiIO.Fonts->AddFontFromFileTTF(path.GetBuffer(), size);

			FW_String iconFontPath = "Data/fa-solid-900.otf";
			FW_FileSystem::GetAbsoluteFilePath(iconFontPath, iconFontPath);

			static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
			ImFontConfig iconFontConfig;
			iconFontConfig.MergeMode = true;
			iconFontConfig.PixelSnapH = true;
			imguiIO.Fonts->AddFontFromFileTTF(iconFontPath.GetBuffer(), size, &iconFontConfig, iconRanges);

			ImGui::SFML::UpdateFontTexture();
		}
	}
}
