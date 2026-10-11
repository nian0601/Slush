#pragma once


// I dont like needing to include Texture here, but its needed due to AssetStorage
// Perhaps I should expand on the IAssetStorage-concept to allow storing AssetStorage
// in places where I dont necessarily care about the concrete asset-type.
// It will make it a bit more cumbersome to use in the places that grabs the storage
// from here, since IAssetStorage will return base Asset*, so it needs to be
// casted into the actual asset, but maybe thats fine?
#include "Core/Assets/AssetParser.h"
#include "Core/Dockables/Dockable.h"
#include "UI/UITheme.h"

namespace Slush
{
	class EntityManager;
	class Font;
}

class ActionGameGlobals
{
public:
	struct DebugSettings
	{
		void LoadFromDisk();
		void SaveToDisk();

		bool myPauseEnemySpawning = false;
		bool mySkipStartScreen = false;
		bool myUseNewUI = false;
		bool myShowPhysicsObjects = false;
		bool myShowPhysicsContacts = false;

	private:
		void OnParse(Slush::AssetParser::Handle aHandle);
	};

	class DebugSettingsDockable : public Slush::Dockable
	{
	public:
		const char* GetName() const override { return "Debug Settings"; }

	protected:
		void OnBuildUI() override;
	};

	static ActionGameGlobals& GetInstance();
	static void Destroy();

	void SetFont(Slush::Font& aFont);
	const Slush::UITheme& GetUITheme() const { return myUITheme; }

	void SetEntityManager(Slush::EntityManager* anEntityManager) { myEntityManager = anEntityManager; }

	Slush::Font& GetFont();

	Slush::EntityManager& GetEntityManager();

	DebugSettings myDebugSettings;

private:
	ActionGameGlobals();
	~ActionGameGlobals();
	static ActionGameGlobals* ourInstance;

	Slush::Font* myFont = nullptr;
	Slush::UITheme myUITheme;

	Slush::EntityManager* myEntityManager = nullptr;
};