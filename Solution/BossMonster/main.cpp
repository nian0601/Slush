#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include <FW_Includes.h>

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

#include "Dockable_CardEditor.h"

class App : public Slush::IApp
{

public:
	void Initialize() override
	{
		Slush::Engine::GetInstance().GetWindow().SetShouldRenderOffscreenBufferToScreen(false);

		FW_GrowingArray<FW_FileSystem::FileInfo> textureInfos;
		FW_FileSystem::GetAllFilesFromRelativeDirectory("Data/Textures", textureInfos);

		for (const FW_FileSystem::FileInfo& info : textureInfos)
			myTextures.Load(info.myFileNameNoExtention.GetBuffer(), info.myRelativeFilePath.GetBuffer());

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

			ImGuiIO& imguiIO = ImGui::GetIO();
			imguiIO.WantCaptureKeyboard = false;
			imguiIO.WantCaptureMouse = false;
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

		myCardEditor.BuildUI(myTextures);

	}

private:
	bool myRenderImGUI = true;

	Slush::Font myFont;
	Slush::AssetStorage<Slush::Texture> myTextures;

	HeroCard* myHeroCard;
	RoomCard* myRoomCard;
	BossCard* myBossCard;

	Dockable_CardEditor myCardEditor;
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