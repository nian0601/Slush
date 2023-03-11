#include "Engine.h"
#include "IApp.h"
#include "imgui/imgui.h"

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
	}

	void Render() override
	{
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
	}

private:
	float x = 0.f;
	float y = 2.f;
	float z = 0.f;
};

int main()
{
	App app;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.Run(app);
	engine.Shutdown();
	
	return 0;
}