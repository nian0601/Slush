#include "stdafx.h"

#include "Core/IApp.h"
#include "Core/Assets/AssetStorage.h"
#include "Core/CommandLineArgs.h"
#include "Core/Engine.h"
#include "Graphics/Window.h"
#include "Graphics/Renderer.h"
#include "Core/Input.h"

#include "Level/Level.h"
#include "Level/LevelData.h"
#include "Level/NavmeshData.h"
#include "Navmesh.h"
#include "NavmeshTestSuite.h"
#include "NavmeshDebuggingLayout.h"
#include "LevelEditorLayout.h"

// Lets Window's "Layouts" menu (re)create a NavmeshDebuggingLayout on demand, without Window needing to
// know it takes a Navmesh& - myNavmesh is a reference into the App-owned Level, which outlives Window.
class NavmeshDebuggingLayoutFactory : public Slush::IAppLayoutFactory
{
public:
	explicit NavmeshDebuggingLayoutFactory(Navmesh& aNavmesh) : myNavmesh(aNavmesh) {}

	const char* GetMenuLabel() const override { return "Navmesh Debugger"; }
	Slush::IAppLayout* CreateLayout() const override { return new NavmeshDebuggingLayout(myNavmesh); }

private:
	Navmesh& myNavmesh;
};

// Same idea for LevelEditorLayout - myLevel is the same App-owned Level referenced above.
class LevelEditorLayoutFactory : public Slush::IAppLayoutFactory
{
public:
	explicit LevelEditorLayoutFactory(Level& aLevel) : myLevel(aLevel) {}

	const char* GetMenuLabel() const override { return "Level Editor"; }
	Slush::IAppLayout* CreateLayout() const override { return new LevelEditorLayout(myLevel); }

private:
	Level& myLevel;
};

class App : public Slush::IApp
{
public:
	void Initialize() override
	{
		Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
		assets.RegisterAssetType<NavmeshData>();
		assets.RegisterAssetType<LevelData>();
		assets.LoadAllAssets();

		myLevel = new Level();

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.RegisterLayout(new NavmeshDebuggingLayoutFactory(myLevel->GetNavmesh()));
		window.RegisterLayout(new LevelEditorLayoutFactory(*myLevel));
		window.SetAppLayout(new LevelEditorLayout(*myLevel));
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myLevel);
	}

	void Update() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();

		if (engine.GetInput().WasKeyReleased(Slush::Input::ESC))
			engine.GetWindow().Close();

		myLevel->Update();
	}

	void Render() override
	{
		Slush::Renderer& renderer = Slush::Engine::GetInstance().GetWindow().GetRenderer();
		renderer.StartOffscreenBuffer();

		myLevel->Render();

		renderer.EndOffscreenBuffer();
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