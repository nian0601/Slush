#pragma once

#include <FW_StaticArray.h>
#include <FW_Vector2.h>

namespace Slush
{
	class Font;

	enum class UIState { Normal, Hovered, Selected, Disabled, Count };
	enum class UITextRole { Title, Header, Body, Small, Count };
	enum class UITextTone { Default, Positive, Warning, Count };

	struct UIVisual
	{
		int myColor = 0;
		int myOutlineColor = 0;
		float myOutlineThickness = 0.f;
	};

	struct UIWidgetStyle
	{
		UIWidgetStyle();

		FW_StaticArray<UIVisual, static_cast<int>(UIState::Count)> myVisuals;
		FW_StaticArray<int, static_cast<int>(UIState::Count)> myTextColors;
		Vector2i myPadding = { 0, 0 };
		int myChildGap = 0;
	};

	struct UITheme
	{
		UITheme();

		Font* myFont = nullptr;
		UIWidgetStyle myPanel;
		UIWidgetStyle myFrame;
		UIWidgetStyle myButton;
		UIWidgetStyle myHeader;
		FW_StaticArray<int, static_cast<int>(UITextRole::Count)> myTextSizes;
		FW_StaticArray<int, static_cast<int>(UITextTone::Count)> myTextColors;
		int myScreenFadeColor = 0xAA121212;
	};

	namespace UIThemeUtils
	{
		UIState ResolveState(UIState aBaseState, bool aIsMouseOver);
		bool IsClickable(UIState aBaseState);
	}
}
