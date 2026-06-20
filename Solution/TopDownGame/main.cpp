#include "stdafx.h"

#include "Core/IApp.h"
#include "Core/Engine.h"
#include "Graphics/Window.h"
#include "Core/Input.h"

#include "Navmesh.h"

class App : public Slush::IApp
{
public:
	void Initialize() override
	{
		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.ToggleEditorUI();
		
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
		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.StartOffscreenBuffer();

		myNavmesh.Render();

		window.EndOffscreenBuffer();
	}

private:
	Navmesh myNavmesh;
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