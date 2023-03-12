#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Core/IApp.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Graphics/Window.h"
#include "Graphics/Sprite.h"

class App : public Slush::IApp
{

public:
	void Initialize() override
	{
	}

	void Shutdown() override
	{
	}

	void Update() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		const Slush::Input& input = engine.GetInput();
		if (input.WasKeyPressed(Slush::Input::ESC))
			engine.GetWindow().Close();

		float delta = Slush::Time::GetDelta();

		float speed = 100.f * delta;
		if (input.IsKeyDown(Slush::Input::W))
			y -= speed;
		if (input.IsKeyDown(Slush::Input::S))
			y += speed;
		if (input.IsKeyDown(Slush::Input::A))
			x -= speed;
		if (input.IsKeyDown(Slush::Input::D))
			x += speed;
	}

	void Render() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		engine.GetWindow().StartOffscreenBuffer();

		mySprite.Render(x, y);

		engine.GetWindow().EndOffscreenBuffer();
	}

	void RenderImGUI() override
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		ImGui::ShowDemoWindow();

		if (ImGui::Begin("Game View"))
		{
			Slush::Engine& engine = Slush::Engine::GetInstance();
			engine.GetWindow().RenderOffscreenBufferToImGUI();
		}
		ImGui::End();
	}

private:
	Slush::Sprite mySprite;

	float x = 200.f;
	float y = 200.f;
	float z = 0.f;
};

int main()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.Initialize();

	App app;
	engine.Run(app);

	engine.Destroy();
	
	return 0;
}