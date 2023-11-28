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

#include "Graphics/Animation/Animation.h"

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

		myCircle = new Slush::CircleSprite();
		myCircle->SetFillColor(0xFFFF0000);
		myCircle->SetOutlineColor(0xFF440000);
		myCircle->SetOutlineThickness(1.f);

		myRect = new Slush::RectSprite();
		myRect->SetFillColor(0xFF00FF00);
		myRect->SetOutlineColor(0xFF004400);
		myRect->SetOutlineThickness(1.f);

		myCircleAnimation = new Slush::Animation(*myCircle);
		myCircleAnimation->myOutlineTrack
			.Wait(0.2f)
			.Linear(0.4f, -1.f, -10.f)
			.Wait(0.2f)
			.Linear(0.4f, -10.f, 4.f)
			.Linear(0.2f, 4, -1.f);
		myCircleAnimation->myScaleTrack
			.Wait(0.2f)
			.Linear(0.6f, 1.f, 1.2f)
			.Wait(0.2f)
			.Linear(0.4f, 1.2f, 1.f);

		myRectAnimation = new Slush::Animation(*myRect);
		myRectAnimation->myOutlineTrack
			.Wait(0.2f)
			.Linear(0.4f, -1.f, -10.f)
			.Wait(0.2f)
			.Linear(0.4f, -10.f, -1.f);
		myRectAnimation->Restart();

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::TextureViewerDockable(myTextures));
		window.AddDockable(new Slush::LogDockable());
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myCircleAnimation);
		FW_SAFE_DELETE(myRectAnimation);
		FW_SAFE_DELETE(myCircle);
		FW_SAFE_DELETE(myRect);
	}

	void Update() override
	{
		myCircleAnimation->Update();
		myRectAnimation->Update();

		Slush::Engine& engine = Slush::Engine::GetInstance();
		if (engine.GetInput().WasKeyPressed(Slush::Input::A))
			myCircleAnimation->Restart();
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
	Slush::Animation* myCircleAnimation;
	Slush::Animation* myRectAnimation;
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