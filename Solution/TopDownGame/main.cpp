#include "stdafx.h"

#include "Core/IApp.h"
#include "Core/Assets/AssetStorage.h"
#include "Core/CommandLineArgs.h"
#include "Core/Engine.h"
#include "Graphics/Window.h"
#include "Core/Input.h"

#include "EntitySystem/EntityManager.h"
#include "EntitySystem/EntityPrefab.h"

#include "Level/Level.h"
#include "Level/LevelData.h"
#include "Level/NavmeshData.h"
#include "Navmesh.h"
#include "NavmeshTestSuite.h"
#include "NavmeshDebuggingLayout.h"
#include "LevelEditorLayout.h"
#include "TopDownGameGlobals.h"

class App : public Slush::IApp
{
public:
	void Initialize() override
	{
		Slush::EntityManager::RegisterComponents();

		Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
		assets.RegisterAssetType<NavmeshData>();
		assets.RegisterAssetType<LevelData>();
		assets.RegisterAssetType<Slush::EntityPrefab>();
		assets.LoadAllAssets();

		myLevel = new Level();

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddLayout(new NavmeshDebuggingLayout());
		window.AddLayout(new LevelEditorLayout(*myLevel), true);
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myLevel);
		TopDownGameGlobals::Destroy();
	}

	void Update() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();

		if (engine.GetInput().WasKeyReleased(Slush::Input::ESC))
			engine.GetWindow().Close();

		myLevel->Update();
	}

private:
	Level* myLevel = nullptr;
};

#include <FW_UnitTestSuite.h>
int main(int argc, char** argv)
{
	FW_UnitTestSuite::RunTests();
	NavmeshTestSuite::RunTests();

	Slush::CommandLineArgs::GetInstance().Parse(argc, argv);

	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.Initialize();

	App app;
	engine.Run(app);

	engine.Destroy();

	return 0;
}
