#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Core/IApp.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Graphics/Window.h"
#include "Graphics/Sprite.h"

#include <SFML/Graphics/RenderTexture.hpp>

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

		if (engine.GetInput().WasKeyPressed(Slush::Input::ESC))
			engine.GetWindow().Close();
	}

	void Render() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		engine.GetWindow().StartOffscreenBuffer();

		mySprite.Render(200, 200);

		engine.GetWindow().EndOffscreenBuffer();
	}

	void RenderImGUI() override
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		ImGui::ShowDemoWindow();

		if (ImGui::Begin("X Window"))
			ImGui::DragFloat("X", &x);
		ImGui::End();

		if (ImGui::Begin("Y Window"))
			ImGui::DragFloat("Y", &y);
		ImGui::End();

		if (ImGui::Begin("Z Window"))
			ImGui::DragFloat("Z", &z);
		ImGui::End();

		if (ImGui::Begin("Game View"))
		{
			Slush::Engine& engine = Slush::Engine::GetInstance();
			float width = ImGui::GetWindowContentRegionWidth();
			float height = (9.f / 16.f) * width;
			ImGui::Image(engine.GetWindow().GetOffscreenBuffer()->getTexture(), { width, height });
		}
		ImGui::End();
		
	}

private:
	float x = 0.f;
	float y = 2.f;
	float z = 0.f;

	Slush::Sprite mySprite;
};

int main()
{
	App app;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.Run(app);
	engine.Shutdown();
	
	return 0;
}