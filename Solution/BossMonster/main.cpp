#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include <FW_Includes.h>

#include "Core/IApp.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/Assets/AssetStorage.h"

#include "Graphics/Window.h"
#include "Graphics/Texture.h"
#include "Graphics/Font.h"

#include "Card.h"
#include "HeroCard.h"
#include "RoomCard.h"
#include "BossCard.h"

#include "CardEditorDockable.h"
#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/TextureViewerDockable.h"
#include "Core/Dockables/LogDockable.h"

class App : public Slush::IApp
{

public:
	void Initialize() override
	{
		myTextures.LoadAllAssets();
		Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
		assets.RegisterAssetType<Slush::Texture>();
		assets.LoadAllAssets();

		myFont.Load("Data/OpenSans-Regular.ttf");

		myHeroCard = new HeroCard(&myFont);
		myHeroCard->Load("Data/Cards/hero_cleric.hero", myTextures);
		myHeroCard->SetPosition(200, 500);

		myRoomCard = new RoomCard(&myFont);
		myRoomCard->Load("Data/Cards/room_spawnpoint.room", myTextures);
		myRoomCard->SetPosition(600, 500);

		myBossCard = new BossCard(&myFont);
		myBossCard->Load("Data/Cards/boss_eclipse.boss", myTextures);
		myBossCard->SetPosition(1000, 500);		

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddDockable(new CardEditorDockable(myTextures));
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::TextureViewerDockable());
		window.AddDockable(new Slush::LogDockable());
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myHeroCard);
		FW_SAFE_DELETE(myRoomCard);
		FW_SAFE_DELETE(myBossCard);
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

		myHeroCard->Render();
		myRoomCard->Render();
		myBossCard->Render();

		engine.GetWindow().EndOffscreenBuffer();
	}

private:
	bool myRenderImGUI = true;

	Slush::Font myFont;
	Slush::AssetStorage<Slush::Texture> myTextures;

	HeroCard* myHeroCard;
	RoomCard* myRoomCard;
	BossCard* myBossCard;
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