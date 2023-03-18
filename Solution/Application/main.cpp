#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Core/IApp.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/AssetStorage.h"

#include "Graphics/Window.h"
#include "Graphics/Sprite.h"
#include "Graphics/Texture.h"
#include "Graphics/Font.h"
#include "Graphics/Text.h"

#include "Card.h"
#include "HeroCard.h"
#include "RoomCard.h"
#include "BossCard.h"
#include <FW_Includes.h>


class App : public Slush::IApp
{

public:
	void Initialize() override
	{
		Slush::Engine::GetInstance().GetWindow().SetShouldRenderOffscreenBufferToScreen(false);
		myTextures.Load("cleric", "Data/cleric.png");
		myTextures.Load("eclipse", "Data/eclipse.png");
		myTextures.Load("spawn_point", "Data/spawn_point.png");

		myFont.Load("Data/OpenSans-Regular.ttf");

		myHeroCard = new HeroCard(
			"Cleric - Ordinary Hero", 
			"~~ Flavor Text ~~", 
			myTextures.GetAsset("cleric"), 
			&myFont, 
			4, 
			2, 
			6);
		myHeroCard->SetPosition(200, 500);

		myRoomCard = new RoomCard(
			"Spawn Point - Monster Room", 
			"Once per turn, if a Hero\ndies in this room, draw two\nRoom cards.", 
			myTextures.GetAsset("spawn_point"), 
			&myFont, 
			5, 
			TreasureType::MAGE);
		myRoomCard->SetPosition(600, 500);

		myBossCard = new BossCard(
			"Eclipse - Charrer Of Souls", 
			"For the rest of the game, when you\ndestroy a Room in your dungeon,\nthe uncovered Room gains +3 until\nthe end of turn.", 
			myTextures.GetAsset("eclipse"), 
			&myFont, 
			600);
		myBossCard->SetPosition(1000, 500);
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myHeroCard);
		FW_SAFE_DELETE(myRoomCard);
		FW_SAFE_DELETE(myBossCard);
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

		if (ImGui::Begin("Asset: Textures"))
		{
			const FW_GrowingArray<Slush::Texture*> textures = myTextures.GetAllAssets();
			for (Slush::Texture* texture : textures)
			{
				if (ImGui::CollapsingHeader(texture->GetAssetName().GetBuffer()))
				{
					ImGui::Text("File: %s", texture->GetFilePath().GetBuffer());
					ImGui::Text("Width: %i, Height: %i", texture->GetSize().x, texture->GetSize().y);
					ImGui::Image(*texture->GetSFMLTexture(), { 100.f, 100.f });
				}
			}

		}
		ImGui::End();

	}

private:
	bool myRenderImGUI = true;

	Slush::Font myFont;
	Slush::AssetStorage<Slush::Texture> myTextures;

	HeroCard* myHeroCard;
	RoomCard* myRoomCard;
	BossCard* myBossCard;
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