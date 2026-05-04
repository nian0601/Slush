#include "stdafx.h"

#include "Core/Assets/AssetStorage.h"
#include "Core/IApp.h"
#include "Core/Input.h"

#include "EntitySystem/EntityManager.h"
#include "EntitySystem/EntityPrefab.h"

#include "Graphics/Animation/Animation.h"
#include "Graphics/Font.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"

#include "Level/LevelData.h"

#include "UI/UILayout.h"

#include "ActionGameGlobals.h"
#include "AssetEditorDockable.h"
#include "CharacterAnimationSet.h"
#include "EntityEditorLayout.h"
#include "GameLayout.h"

class App : public Slush::IApp
{
public:
	void Initialize() override
	{
		EntityManager::RegisterComponents();

		Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
		assets.RegisterAssetType<Slush::UILayout>();
		assets.RegisterAssetType<Slush::Texture>();
		assets.RegisterAssetType<LevelData>();
		assets.RegisterAssetType<Slush::Animation>();
		assets.RegisterAssetType<CharacterAnimationSet>();
		assets.RegisterAssetType<EntityPrefab>();
		assets.LoadAllAssets();

		myFont.Load("Data/NotoSans.ttf");
		ActionGameGlobals::GetInstance().SetFont(myFont);

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		//window.SetAppLayout("Game");
		//myAppLayout = new GameLayout();
		window.SetAppLayout("Entity");
		myAppLayout = new EntityEditorLayout();
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
		else if (input.WasKeyReleased(Slush::Input::KeyCode::_3))
		{
			SLUSH_INFO("SOME INFO");
		}
		else if (input.WasKeyReleased(Slush::Input::KeyCode::_4))
		{
			SLUSH_DEBUG("SOME DEBUG");
		}
		else if (input.WasKeyReleased(Slush::Input::KeyCode::_5))
		{
			SLUSH_WARNING("SOME WARNING");
		}
		else if (input.WasKeyReleased(Slush::Input::KeyCode::_6))
		{
			SLUSH_ERROR("SOME ERROR");
		}



		myAppLayout->Update();
	}

	void Render() override
	{
		myAppLayout->Render();
	}

private:
	Slush::Font myFont;
	AppLayout* myAppLayout = nullptr;
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