#pragma once


// I dont like needing to include Texture here, but its needed due to AssetStorage
// Perhaps I should expand on the IAssetStorage-concept to allow storing AssetStorage
// in places where I dont necessarily care about the concrete asset-type.
// It will make it a bit more cumbersome to use in the places that grabs the storage
// from here, since IAssetStorage will return base Asset*, so it needs to be
// casted into the actual asset, but maybe thats fine?
#include "Core/Assets/AssetParser.h"
#include "Core/Dockables/Dockable.h"

class EntityManager;
namespace Slush
{
	class Font;
	class PhysicsWorld;
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

	void SetFont(Slush::Font& aFont) { myFont = &aFont; }

	void SetEntityManager(EntityManager* anEntityManager) { myEntityManager = anEntityManager; }
	void SetPhysicsWorld(Slush::PhysicsWorld* aPhysicsWorld) { myPhysicsWorld = aPhysicsWorld; }

	Slush::Font& GetFont();

	EntityManager& GetEntityManager();
	Slush::PhysicsWorld& GetPhysicsWorld();

	DebugSettings myDebugSettings;

private:
	ActionGameGlobals();
	~ActionGameGlobals();
	static ActionGameGlobals* ourInstance;

	Slush::Font* myFont = nullptr;

	EntityManager* myEntityManager = nullptr;
	Slush::PhysicsWorld* myPhysicsWorld = nullptr;
};