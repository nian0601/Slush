#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include <FW_Includes.h>

#include "Core/IApp.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/Assets/AssetStorage.h"

#include "Core/Dockables/LogDockable.h"
#include "Graphics/Window.h"


#include "FilterMakerDockable.h"
#include "FilterColorHelperDockable.h"

class App : public Slush::IApp
{

public:
	void Initialize() override
	{
		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddDockable(new Slush::LogDockable());
		window.AddDockable(new FilterMakerDockable());
		window.AddDockable(new FilterColorHelperDockable());
	}

	void Shutdown() override
	{
	}

	void Update() override
	{
		float delta = Slush::Time::GetDelta();
		delta;
	}

	void Render() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		engine.GetWindow().StartOffscreenBuffer();


		engine.GetWindow().EndOffscreenBuffer();
	}

private:
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