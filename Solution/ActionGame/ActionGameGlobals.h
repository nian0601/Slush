#pragma once


// I dont like needing to include Texture here, but its needed due to AssetStorage
// Perhaps I should expand on the IAssetStorage-concept to allow storing AssetStorage
// in places where I dont necessarily care about the concrete asset-type.
// It will make it a bit more cumbersome to use in the places that grabs the storage
// from here, since IAssetStorage will return base Asset*, so it needs to be
// casted into the actual asset, but maybe thats fine?
#include "Graphics/Texture.h"
#include "Core/Assets/AssetParser.h"
#include "Core/Dockables/Dockable.h"
#include "UI/UILayout.h"

class EntityManager;
namespace Slush
{
	template<typename T>
	class AssetStorage;

	class Font;
	class Texture;
	class PhysicsWorld;
	struct UILayout;
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

	void SetTextureStorage(Slush::AssetStorage<Slush::Texture>& aStorage) { myTextureStorage = &aStorage; }
	void SetEntityPrefabStorage(Slush::AssetStorage<EntityPrefab>& aStorage) { myEntityPrefabStorage = &aStorage; }
	void SetUILayoutStorage(Slush::AssetStorage<Slush::UILayout>& aStorage) { myUILayoutStorage = &aStorage; }
	void SetFont(Slush::Font& aFont) { myFont = &aFont; }

	void SetEntityManager(EntityManager* anEntityManager) { myEntityManager = anEntityManager; }
	void SetPhysicsWorld(Slush::PhysicsWorld* aPhysicsWorld) { myPhysicsWorld = aPhysicsWorld; }

	Slush::AssetStorage<Slush::Texture>& GetTextureStorage();
	Slush::AssetStorage<EntityPrefab>& GetEntityPrefabStorage();
	Slush::AssetStorage<Slush::UILayout>& GetUILayoutStorage();
	Slush::Font& GetFont();

	EntityManager& GetEntityManager();
	Slush::PhysicsWorld& GetPhysicsWorld();

	DebugSettings myDebugSettings;

private:
	ActionGameGlobals();
	~ActionGameGlobals();
	static ActionGameGlobals* ourInstance;

	Slush::AssetStorage<Slush::Texture>* myTextureStorage = nullptr;
	Slush::AssetStorage<Slush::UILayout>* myUILayoutStorage = nullptr;
	Slush::AssetStorage<EntityPrefab>* myEntityPrefabStorage = nullptr;
	Slush::Font* myFont = nullptr;

	EntityManager* myEntityManager = nullptr;
	Slush::PhysicsWorld* myPhysicsWorld = nullptr;
};