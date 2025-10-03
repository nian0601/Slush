#include "stdafx.h"

#include "Core/IApp.h"
#include "Core/Assets/AssetStorage.h"

#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/TextureViewerDockable.h"
#include "Core/Dockables/LogDockable.h"
#include "Core/Dockables/ContentBrowserDockable.h"

#include "Graphics/Window.h"
#include "Graphics/Font.h"

#include "ActionGameGlobals.h"
#include "EntityPrefabDockable.h"
#include "EntityManager.h"
#include "GameLayout.h"

class App : public Slush::IApp
{
public:
	void Initialize() override
	{
		EntityManager::RegisterComponents();

		myTextures.LoadAllAssets();
		myEntityPrefabs.LoadAllAssets();
		myUILayouts.LoadAllAssets();

		myFont.Load("Data/OpenSans-Regular.ttf");

		ActionGameGlobals::GetInstance().SetTextureStorage(myTextures);
		ActionGameGlobals::GetInstance().SetEntityPrefabStorage(myEntityPrefabs);
		ActionGameGlobals::GetInstance().SetUILayoutStorage(myUILayouts);
		ActionGameGlobals::GetInstance().SetFont(myFont);

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::LogDockable());
		window.AddDockable(new ActionGameGlobals::DebugSettingsDockable());
		//window.AddDockable(new Slush::TextureViewerDockable(myTextures));
		//window.AddDockable(new Slush::UIEditorDockable(myUILayouts));
		//window.AddDockable(new EntityPrefabDockable(myEntityPrefabs));

		Slush::ContentBrowserDockable* contentBrowser = new Slush::ContentBrowserDockable();
		window.AddDockable(contentBrowser);

		contentBrowser->AddAssetStorage(&myEntityPrefabs);
		contentBrowser->AddAssetStorage(&myTextures);
		contentBrowser->AddAssetStorage(&myUILayouts);

		myGameLayout = new GameLayout();
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myGameLayout);

		ActionGameGlobals::Destroy();
	}

	void Update() override
	{
		myGameLayout->Update();
	}

	void Render() override
	{
		myGameLayout->Render();
	}

private:
	Slush::Font myFont;
	Slush::AssetStorage<Slush::Texture> myTextures;
	Slush::AssetStorage<EntityPrefab> myEntityPrefabs;
	Slush::AssetStorage<Slush::UILayout> myUILayouts;

	GameLayout* myGameLayout;
};

#include <FW_UnitTestSuite.h>
int main()
{
	FW_UnitTestSuite::RunTests();

	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.Initialize();

	App app;
	engine.Run(app);

	engine.Destroy();
	
	return 0;
}