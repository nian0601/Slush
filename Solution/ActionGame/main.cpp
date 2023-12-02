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
		myCircleAnimation->myScaleTrack
			.Linear(0.2f, 1.f, 0.1f)
			.Wait(0.1f)
			.Linear(0.2f, 0.1f, 1.f);
		myCircleAnimation->myPositionTrack
			.Wait(0.2f)
			.Constant(1.2f, 1.f);
		myCircleAnimation->MakeOneShot();

		myRectAnimation = new Slush::Animation(*myRect);
		myRectAnimation->myOutlineTrack
			.Wait(0.2f)
			.Linear(0.4f, -1.f, -10.f)
			.Wait(0.2f)
			.Linear(0.4f, -10.f, -1.f);
		myRectRuntime.Restart();

		myCirclePosition = { 400.f, 400.f };
		myRectPosition = { 700.f, 400.f };

		myCircleRuntime.myStartPosition = myCirclePosition;
		myCircleRuntime.myEndPosition = myCirclePosition + Vector2f(200.f, 0.f);
		myCircleRuntime.myCurrentPosition = myCirclePosition;

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
		myCircleAnimation->Update(myCircleRuntime);
		myRectAnimation->Update(myRectRuntime);

		Slush::Engine& engine = Slush::Engine::GetInstance();
		if (engine.GetInput().WasKeyPressed(Slush::Input::A))
		{
			myCircleRuntime.myStartPosition = myCircleRuntime.myCurrentPosition;
			myCircleRuntime.myEndPosition = engine.GetInput().GetMousePositionf();
			myCircleRuntime.Restart();
		}

		if (engine.GetInput().IsMouseDown(Slush::Input::LEFTMB))
		{
			myCircleRuntime.myStartPosition = myCircleRuntime.myCurrentPosition;
			myCircleRuntime.myEndPosition = engine.GetInput().GetMousePositionf();
			myCircleRuntime.Restart();
		}
	}

	void Render() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		engine.GetWindow().StartOffscreenBuffer();

		myCircle->Render(myCircleRuntime.myCurrentPosition.x, myCircleRuntime.myCurrentPosition.y);
		myRect->Render(myRectPosition.x, myRectPosition.y);

		engine.GetWindow().EndOffscreenBuffer();
	}

private:
	Slush::Font myFont;
	Slush::AssetStorage<Slush::Texture> myTextures;

	Slush::CircleSprite* myCircle;
	Slush::RectSprite* myRect;
	Slush::Animation* myCircleAnimation;
	Slush::Animation* myRectAnimation;

	Slush::AnimationRuntime myCircleRuntime;
	Slush::AnimationRuntime myRectRuntime;

	Vector2f myCirclePosition;
	Vector2f myRectPosition;
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