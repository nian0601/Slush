#pragma once

#include "UI/UITheme.h"

class Level;
namespace Slush { class Font; }

class TopDownGameGlobals
{
public:
	static TopDownGameGlobals& GetInstance();
	static void Destroy();

	void SetLevel(Level* aLevel) { myLevel = aLevel; }
	Level& GetLevel();
	void SetFont(Slush::Font& aFont);
	const Slush::UITheme& GetUITheme() const { return myUITheme; }
	Slush::Font& GetFont();

private:
	TopDownGameGlobals() = default;
	~TopDownGameGlobals() = default;

	static TopDownGameGlobals* ourInstance;

	Level* myLevel = nullptr;
	Slush::Font* myFont = nullptr;
	Slush::UITheme myUITheme;
};
