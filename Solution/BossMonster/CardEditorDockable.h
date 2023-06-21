#pragma once

#include <FW_String.h>
#include <FW_FileSystem.h>

#include "Core/Dockables/Dockable.h"

#include "Card.h"

namespace Slush
{
	class Texture;
}

class CardEditorDockable : public Slush::Dockable
{
public:
	CardEditorDockable(const Slush::AssetStorage<Slush::Texture>& someTextures);

	const char* GetName() const override { return "Card Editor"; }

protected:
	void OnBuildUI() override;

private:
	void SaveCard();
	void LoadCard(const FW_String& aFilePath);

	FW_FileSystem::FileInfo myCardsDirectoryInfo;
	FW_String myCardsFilePath;
	const Slush::AssetStorage<Slush::Texture>& myCardTextures;

	enum CardType
	{
		NONE,
		ROOM,
		BOSS,
		HERO,
		SPELL,
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