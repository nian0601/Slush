#include "stdafx.h"

#include "Core/IApp.h"
#include "Core/Assets/AssetStorage.h"

#include "Graphics/Font.h"

#include "ActionGameGlobals.h"
#include "AssetEditorDockable.h"
#include "EntitySystem/EntityManager.h"
#include "GameLayout.h"
#include "EntityEditorLayout.h"
#include <Core/Input.h>
#include <Graphics/Window.h>

#include "Level/LevelData.h"
#include <EntitySystem/EntityPrefab.h>
#include <UI/UILayout.h>
#include <Graphics/Texture.h>
#include <Graphics/Animation/Animation.h>

class App : public Slush::IApp
{
public:
	void Initialize() override
	{
		EntityManager::RegisterComponents();
	
		Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
		assets.RegisterAssetType<EntityPrefab>();
		assets.RegisterAssetType<Slush::UILayout>();
		assets.RegisterAssetType<Slush::Texture>();
		assets.RegisterAssetType<Slush::Animation>();
		assets.RegisterAssetType<LevelData>();

		assets.LoadAllAssets();

		myFont.Load("Data/NotoSans.ttf");
		ActionGameGlobals::GetInstance().SetFont(myFont);

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.SetAppLayout("Game");
		myAppLayout = new GameLayout();
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myAppLayout);

		ActionGameGlobals::Destroy();
	}

	void Update() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		const Slush::Input& input = engine.GetInput();
		if (input.WasKeyReleased(Slush::Input::KeyCode::_1))
		{
			FW_SAFE_DELETE(myAppLayout);
			Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
			window.SetAppLayout("Game");
			myAppLayout = new GameLayout();
		}
		else if (input.WasKeyReleased(Slush::Input::KeyCode::_2))
		{
			FW_SAFE_DELETE(myAppLayout);
			Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
			window.SetAppLayout("Entity");
			myAppLayout = new EntityEditorLayout();
		}


		myAppLayout->Update();
	}

	void Render() override
	{
		myAppLayout->Render();
	}

private:
	Slush::Font myFont;
	AppLayout* myAppLayout;
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