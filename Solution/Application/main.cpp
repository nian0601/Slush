#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Core/IApp.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"

#include "Graphics/Window.h"
#include "Graphics/Sprite.h"
#include "Graphics/Texture.h"
#include "Graphics/Font.h"
#include "Graphics/Text.h"

class App : public Slush::IApp
{

public:
	void Initialize() override
	{
		Slush::Engine::GetInstance().GetWindow().SetShouldRenderOffscreenBufferToScreen(false);
		myTexture.Load("Data/cleric.PNG");
		mySprite.SetTexture(myTexture);

		myFont.Load("Data/OpenSans-Regular.ttf");
		myText.SetFont(myFont);
		myText.SetText("This is some kewl text");
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

		if (input.WasKeyPressed(Slush::Input::HYPHEN))
		{
			myRenderImGUI = !myRenderImGUI;
			engine.GetWindow().SetShouldRenderOffscreenBufferToScreen(!myRenderImGUI);
		}

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
		mySprite.Render(1700.f, 200.f);
		mySprite.Render(1700.f, 800.f);
		mySprite.Render(200.f, 800.f);

		myText.Render(980.f, 560.f);

		engine.GetWindow().EndOffscreenBuffer();
	}

	void RenderImGUI() override
	{
		if (!myRenderImGUI)
			return;

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
	bool myRenderImGUI = true;
	Slush::Sprite mySprite;
	Slush::Texture myTexture;

	Slush::Text myText;
	Slush::Font myFont;

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