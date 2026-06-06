#include "stdafx.h"

#include "Core/IApp.h"
#include "Core/Engine.h"



class App : public Slush::IApp
{
public:
	void Initialize() override
	{
		//EntityManager::RegisterComponents();
		//
		//Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
		//assets.RegisterAssetType<Slush::UILayout>();
		//assets.RegisterAssetType<Slush::Texture>();
		//assets.RegisterAssetType<LevelData>();
		//assets.RegisterAssetType<Slush::Animation>();
		//assets.RegisterAssetType<CharacterAnimationSet>();
		//assets.RegisterAssetType<CharacterInfo>();
		//assets.RegisterAssetType<EntityPrefab>();
		//assets.RegisterAssetType<WeaponData>();
		//assets.RegisterAssetType<StatsUpgradeData>();
		//assets.LoadAllAssets();
		//
		//myFont.Load("Data/NotoSans.ttf");
		//ActionGameGlobals::GetInstance().SetFont(myFont);
		//
		//Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		//window.SetAppLayout("Game");
		//myAppLayout = new GameLayout();
		//window.SetAppLayout("Entity");
		//myAppLayout = new EntityEditorLayout();
	}

	void Shutdown() override
	{
		//FW_SAFE_DELETE(myAppLayout);

		//ActionGameGlobals::Destroy();
	}

	void Update() override
	{
		//Slush::Engine& engine = Slush::Engine::GetInstance();
		//const Slush::Input& input = engine.GetInput();
		//if (input.WasKeyReleased(Slush::Input::KeyCode::_1))
		//{
		//	FW_SAFE_DELETE(myAppLayout);
		//	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		//	window.SetAppLayout("Game");
		//	myAppLayout = new GameLayout();
		//}
		//else if (input.WasKeyReleased(Slush::Input::KeyCode::_2))
		//{
		//	FW_SAFE_DELETE(myAppLayout);
		//	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		//	window.SetAppLayout("Entity");
		//	myAppLayout = new EntityEditorLayout();
		//}
		//else if (input.WasKeyReleased(Slush::Input::KeyCode::_3))
		//{
		//	SLUSH_INFO("SOME INFO");
		//}
		//else if (input.WasKeyReleased(Slush::Input::KeyCode::_4))
		//{
		//	SLUSH_DEBUG("SOME DEBUG");
		//}
		//else if (input.WasKeyReleased(Slush::Input::KeyCode::_5))
		//{
		//	SLUSH_WARNING("SOME WARNING");
		//}
		//else if (input.WasKeyReleased(Slush::Input::KeyCode::_6))
		//{
		//	SLUSH_ERROR("SOME ERROR");
		//}
		//
		//
		//
		//myAppLayout->Update();
	}

	void Render() override
	{
		//myAppLayout->Render();
	}

private:
	//Slush::Font myFont;
	//AppLayout* myAppLayout = nullptr;
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