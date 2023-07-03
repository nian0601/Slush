#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include <FW_Includes.h>

#include "Core/IApp.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/AssetStorage.h"

#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/TextureViewerDockable.h"
#include "Core/Dockables/LogDockable.h"

#include "Graphics/Window.h"
#include "Graphics/Texture.h"
#include "Graphics/Font.h"
#include "Graphics/CircleSprite.h"
#include "Graphics/RectSprite.h"

class App : public Slush::IApp
{

public:
	void Initialize() override
	{
		FW_GrowingArray<FW_FileSystem::FileInfo> textureInfos;
		FW_FileSystem::GetAllFilesFromRelativeDirectory("Data/Textures", textureInfos);

		for (const FW_FileSystem::FileInfo& info : textureInfos)
			myTextures.Load(info.myFileNameNoExtention.GetBuffer(), info.myRelativeFilePath.GetBuffer());

		myFont.Load("Data/OpenSans-Regular.ttf");

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::TextureViewerDockable(myTextures));
		window.AddDockable(new Slush::LogDockable());

		myCircle = new Slush::CircleSprite();
		myCircle->SetFillColor(0xFFFF0000);
		myCircle->SetOutlineColor(0xFF440000);

		myRect = new Slush::RectSprite();
		myRect->SetFillColor(0xFF00FF00);
		myRect->SetOutlineColor(0xFF004400);
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myCircle);
		FW_SAFE_DELETE(myRect);
	}

	void Update() override
	{
		float delta = Slush::Time::GetDelta();
		myTimer += delta * 10;
		myThickness = -8.f + (sin(myTimer) - 1.f) * 5.f;

		myCircle->SetOutlineThickness(myThickness);
		myRect->SetOutlineThickness(myThickness);
	}

	void Render() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		engine.GetWindow().StartOffscreenBuffer();

		myCircle->Render(400.f, 400.f);
		myRect->Render(700.f, 400.f);

		engine.GetWindow().EndOffscreenBuffer();
	}

private:
	Slush::Font myFont;
	Slush::AssetStorage<Slush::Texture> myTextures;
	Slush::CircleSprite* myCircle;
	Slush::RectSprite* myRect;
	float myThickness = 0.f;
	float myTimer = 0.f;
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