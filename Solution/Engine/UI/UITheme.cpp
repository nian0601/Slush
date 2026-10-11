#include "stdafx.h"
#include "UITheme.h"

namespace Slush
{
	UIWidgetStyle::UIWidgetStyle()
	{
		myVisuals.Fill(UIVisual());
		myTextColors.Fill(0xFFFFFFFF);
	}

	UITheme::UITheme()
	{
		myPanel.myVisuals.Fill({ static_cast<int>(0xAA121212), 0, 0.f });
		myPanel.myPadding = { 16, 16 };
		myPanel.myChildGap = 16;
		myHeader = myPanel;

		myButton.myPadding = { 16, 16 };
		myButton.myChildGap = 8;
		myButton.myVisuals[static_cast<int>(UIState::Normal)] = { static_cast<int>(0xFF333333), static_cast<int>(0xFF000000), -1.f };
		myButton.myVisuals[static_cast<int>(UIState::Hovered)] = { static_cast<int>(0xFF888888), static_cast<int>(0xFF000000), -1.f };
		myButton.myVisuals[static_cast<int>(UIState::Selected)] = { static_cast<int>(0xFF666666), static_cast<int>(0xFFFFFFFF), -1.f };
		myButton.myVisuals[static_cast<int>(UIState::Disabled)] = { static_cast<int>(0xFF222222), static_cast<int>(0xFF000000), -1.f };
		myButton.myTextColors[static_cast<int>(UIState::Disabled)] = 0xFF888888;

		myFrame = myButton;
		myFrame.myVisuals.Fill(myButton.myVisuals[static_cast<int>(UIState::Normal)]);
		myFrame.myTextColors.Fill(0xFFFFFFFF);

		myTextSizes[static_cast<int>(UITextRole::Title)] = 50;
		myTextSizes[static_cast<int>(UITextRole::Header)] = 32;
		myTextSizes[static_cast<int>(UITextRole::Body)] = 25;
		myTextSizes[static_cast<int>(UITextRole::Small)] = 18;
		myTextColors[static_cast<int>(UITextTone::Default)] = 0xFFFFFFFF;
		myTextColors[static_cast<int>(UITextTone::Positive)] = 0xFF44FF44;
		myTextColors[static_cast<int>(UITextTone::Warning)] = 0xFFFFD17A;
	}

	namespace UIThemeUtils
	{
		UIState ResolveState(UIState aBaseState, bool aIsMouseOver)
		{
			return aBaseState == UIState::Normal && aIsMouseOver ? UIState::Hovered : aBaseState;
		}

		bool IsClickable(UIState aBaseState)
		{
			return aBaseState != UIState::Disabled;
		}
	}
}
