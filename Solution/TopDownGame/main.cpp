#include "stdafx.h"

#include "Core/IApp.h"
#include "Core/Assets/AssetStorage.h"
#include "Core/CommandLineArgs.h"
#include "Core/Engine.h"
#include "Graphics/Window.h"
#include "Core/Input.h"
#include "Graphics/Font.h"

#include "EntitySystem/EntityManager.h"
#include "EntitySystem/EntityPrefab.h"

#include "Level/LevelData.h"
#include "Level/NavmeshData.h"
#include "Navmesh.h"
#include "NavmeshTestSuite.h"
#include "WaveScalingTestSuite.h"
#include "GameLayout.h"
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
		myFont.Load("Fonts/NotoSans.ttf");
		TopDownGameGlobals::GetInstance().SetFont(myFont);

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddLayout(new GameLayout(), true);
		window.AddLayout(new NavmeshDebuggingLayout());
		window.AddLayout(new LevelEditorLayout());
	}

	void Shutdown() override
	{
		TopDownGameGlobals::Destroy();
	}

	void Update() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();

		if (engine.GetInput().WasKeyReleased(Slush::Input::ESC))
			engine.GetWindow().Close();

	}

private:
	Slush::Font myFont;
};

#include "Core/AssetRepository.h"
#include "Core/UnitTests.h"
static void RunGameTests()
{
	NavmeshTestSuite::RunTests();
	WaveScalingTestSuite::RunTests();
}

int main(int argc, char** argv)
{
	Slush::CommandLineArgs::GetInstance().Parse(argc, argv);
	Slush::AssetRepository::Mount("TopDownGame");

	const Slush::UnitTests::Outcome testOutcome = Slush::UnitTests::Run(&RunGameTests);
	if (testOutcome != Slush::UnitTests::Outcome::Continue)
		return testOutcome == Slush::UnitTests::Outcome::ExitPassed ? 0 : 1;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.Initialize();

	App app;
	engine.Run(app);

	engine.Destroy();

	return 0;
}
