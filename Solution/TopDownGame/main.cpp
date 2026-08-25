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
		window.SetAppLayout(new NavmeshDebuggingLayout(myLevel->GetNavmesh()));
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