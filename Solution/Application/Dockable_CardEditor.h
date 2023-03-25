#pragma once

#include <FW_String.h>

#include "Card.h"

class Dockable_CardEditor
{
public:
	Dockable_CardEditor();
	void BuildUI();

private:
	enum CardType
	{
		NONE,
		ROOM,
		BOSS,
		HERO,
	};

	int myCardType = NONE;
	FW_String myFileName;

	//All cards
	FW_String myTitle;
	FW_String myDescription;
	FW_String myTextureName;

	//Room
	int myDamageValue = 0;
	int myTreasureType = CLERIC;

	//Boss
	int myXPValue = 0;

	//Hero
	int myHealthValue = 0;
	int myWoundValue = 0;
	int mySoulValue = 0;
};