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
		myLevelEditorLayout = new LevelEditorLayout();
		window.AddLayout(myLevelEditorLayout);

		Slush::CommandLineArgs& commandLineArgs = Slush::CommandLineArgs::GetInstance();
		if (commandLineArgs.HasFlag("-layout"))
		{
			const char* layoutName = commandLineArgs.GetString("-layout");
			if (!layoutName)
			{
				SLUSH_WARNING("Missing -layout value; keeping the default layout");
			}
			else if (!window.ActivateLayoutByName(layoutName))
			{
				SLUSH_WARNING("Unknown -layout '%s'; keeping the default layout", layoutName);
			}
		}
	}

	void Shutdown() override
	{
		TopDownGameGlobals::Destroy();
	}

	void Update() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();

		// The Level Editor uses Esc itself (cancel the active tool), so it never closes the app there.
		Slush::Window& window = engine.GetWindow();
		if (engine.GetInput().WasKeyReleased(Slush::Input::ESC) && window.GetActiveLayout() != myLevelEditorLayout)
			window.Close();

	}

private:
	Slush::Font myFont;
	LevelEditorLayout* myLevelEditorLayout = nullptr;
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
	const int exitCode = engine.Run(app);

	engine.Destroy();

	return exitCode;
}
