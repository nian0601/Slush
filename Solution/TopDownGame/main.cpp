#include "stdafx.h"

#include "Core/IApp.h"
#include "Core/Assets/AssetStorage.h"
#include "Core/CommandLineArgs.h"
#include "Core/Dockables/AssetEditorLayout.h"
#include "Core/Engine.h"
#include "Graphics/Window.h"
#include "Graphics/Renderer.h"
#include "Core/Input.h"

#include "Level/NavmeshData.h"
#include "Navmesh.h"
#include "NavmeshTestSuite.h"

class App : public Slush::IApp
{
public:
	void Initialize() override
	{
		Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
		assets.RegisterAssetType<NavmeshData>();
		assets.LoadAllAssets();

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.ToggleEditorUI();
		window.SetAppLayout(new Slush::AssetEditorLayout());

		myNavmesh.GenerateDefaultGrid();
	}

	void Shutdown() override
	{
	}

	void Update() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();

		if (engine.GetInput().WasKeyReleased(Slush::Input::ESC))
			engine.GetWindow().Close();

		myNavmesh.Update();
	}

	void Render() override
	{
		Slush::Renderer& renderer = Slush::Engine::GetInstance().GetWindow().GetRenderer();
		renderer.StartOffscreenBuffer();

		myNavmesh.Render();

		renderer.EndOffscreenBuffer();
	}

private:
	Navmesh myNavmesh;
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