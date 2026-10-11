#include "stdafx.h"

#include "TopDownGameGlobals.h"

#include "Level/Level.h"

TopDownGameGlobals* TopDownGameGlobals::ourInstance = nullptr;

TopDownGameGlobals& TopDownGameGlobals::GetInstance()
{
	if (!ourInstance)
	{
		ourInstance = new TopDownGameGlobals();
	}

	return *ourInstance;
}

void TopDownGameGlobals::Destroy()
{
	FW_SAFE_DELETE(ourInstance);
}

Level& TopDownGameGlobals::GetLevel()
{
	FW_ASSERT(myLevel != nullptr, "Need to set a Level");
	return *myLevel;
}

Slush::Font& TopDownGameGlobals::GetFont()
{
	FW_ASSERT(myFont != nullptr, "Need to set a Font");
	return *myFont;
}

void TopDownGameGlobals::SetFont(Slush::Font& aFont)
{
	myFont = &aFont;
	myUITheme = Slush::UITheme();
	myUITheme.myFont = &aFont;
	myUITheme.myTextSizes[static_cast<int>(Slush::UITextRole::Body)] = 18;
	myUITheme.myPanel.myVisuals.Fill({ static_cast<int>(0xDD18202A), 0, 0.f });
	myUITheme.myHeader = myUITheme.myPanel;
	myUITheme.myTooltip.myVisuals.Fill({ static_cast<int>(0xF018202A), static_cast<int>(0xFF81C9FF), -1.f });

	Slush::UIWidgetStyle& button = myUITheme.myButton;
	button.myVisuals[static_cast<int>(Slush::UIState::Normal)] = { static_cast<int>(0xFF334453), 0, 0.f };
	button.myVisuals[static_cast<int>(Slush::UIState::Hovered)] = { static_cast<int>(0xFF60758A), 0, 0.f };
	button.myVisuals[static_cast<int>(Slush::UIState::Selected)] = { static_cast<int>(0xFF4379A6), static_cast<int>(0xFF81C9FF), 2.f };
	button.myVisuals[static_cast<int>(Slush::UIState::Disabled)] = { static_cast<int>(0xFF242D36), 0, 0.f };
	myUITheme.myFrame = button;
	myUITheme.myFrame.myVisuals.Fill(button.myVisuals[static_cast<int>(Slush::UIState::Normal)]);
	myUITheme.myFrame.myTextColors.Fill(0xFFFFFFFF);
}
