#include "stdafx.h"

#include "Graphics/Window.h"
#include "Graphics/Renderer.h"
#include "Core/Log.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/Dockables/Dockable.h"
#include "Core/Dockables/IAppLayout.h"


#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include <FW_FileSystem.h>

namespace Slush
{
	Window::Window(unsigned int aWidth, unsigned int aHeight)
		: myAspectRatio(16.f/9.f)
	{
		myWindowRect = MakeRectFromTopLeft<float>({ 0.f, 0.f }, { static_cast<float>(aWidth), static_cast<float>(aHeight) });
		myGameViewRect = MakeRectFromTopLeft<float>({ 0.f, 0.f }, GetSizeThatRespectsAspectRatio(aWidth, aHeight));

		myRenderWindow = new sf::RenderWindow(sf::VideoMode({ aWidth, aHeight }), "Slush Engine");
		myRenderer = new Renderer(myRenderWindow);

		ImGui::SFML::Init(*myRenderWindow, false);
		
		ImGuiIO& imguiIO = ImGui::GetIO();
		imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		imguiIO.IniFilename = NULL;// path.GetBuffer();

		

		// Having this enabled makes it tricky/impossible to have Keyboard-input for the gameview :/
		// An option could be to have this be togglable somehow maybe?
		//imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		imguiIO.ConfigWindowsMoveFromTitleBarOnly = true;
		SLUSH_INFO("Window Created");
	}

	Window::~Window()
	{
		SaveAppLayoutConfig();

		FW_SAFE_DELETE(myRenderer);
		FW_SAFE_DELETE(myRenderWindow);
	}

	void Window::Hide()
	{
		myRenderWindow->setVisible(false);
	}

	bool Window::PumpEvents()
	{
		if (!myShouldBeOpen)
			return false;

		while (const std::optional event = myRenderWindow->pollEvent())
		{
			ImGui::SFML::ProcessEvent(*myRenderWindow, *event);

			if (event->is<sf::Event::Closed>())
			{
				Close();
			}
			else if (const auto* resized = event->getIf<sf::Event::Resized>())
			{
				SetRectSize(myWindowRect, { static_cast<float>(resized->size.x), static_cast<float>(resized->size.y) });
				//myGameViewRect = MakeRectFromTopLeft<float>({ 0.f, 0.f }, GetSizeThatRespectsAspectRatio(aWidth, aHeight));
				//myRenderWindow->setSize({ event.size.width, event.size.height });
				sf::FloatRect visibleArea({ 0.f, 0.f }, { static_cast<float>(resized->size.x), static_cast<float>(resized->size.y) });
				myRenderWindow->setView(sf::View(visibleArea));
			}
		}

		if (Slush::Engine::GetInstance().GetInput().WasKeyPressed(Slush::Input::HYPHEN))
			ToggleEditorUI();

		if (myShowEditorUI)
			ImGui::SFML::Update(*myRenderWindow, Time::GetDelta());

		return true;
	}

	void Window::RenderOffscreenBufferToImGUI()
	{
		ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
		ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
		float width = regionMax.x - regionMin.x;
		float height = regionMax.y - regionMin.y;

		ImVec2 windowPos = ImGui::GetWindowPos();
		windowPos.x += regionMin.x;
		windowPos.y += regionMin.y;

		myGameViewRect = MakeRectFromTopLeft({ windowPos.x, windowPos.y }, GetSizeThatRespectsAspectRatio(static_cast<int>(width), static_cast<int>(height)));

		ImTextureID textureID = myRenderer->GetOffscreenBuffer()->getTexture().getNativeHandle();
		ImGui::Image(textureID, { myGameViewRect.myExtents.x, myGameViewRect.myExtents.y }, { 0, 1 }, { 1, 0 });
	}

	void Window::Present()
	{
		UpdatePendingClose();

		if (myShowEditorUI)
		{
			if (ImGui::BeginMainMenuBar())
			{
				if (myAppLayout)
					ImGui::Text("[ %s ]", myAppLayout->GetName().GetBuffer());

				if (ImGui::BeginMenu("Layouts"))
				{
					ImGui::Selectable("Game");
					ImGui::Selectable("Entity");
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("ImGUI"))
				{
					ImGui::Checkbox("Show Demo", &myDisplayImGUIDemo);
					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}

			//ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
			ImGui::DockSpaceOverViewport();

			if (myDisplayImGUIDemo)
				ImGui::ShowDemoWindow(&myDisplayImGUIDemo);

			if (myAppLayout)
				myAppLayout->Update();

			ImGui::SFML::Render(*myRenderWindow);
		}
		else
		{
			myGameViewRect = myWindowRect;

			// If we're not in 'ShowEditorUI'-mode, then we need to render the OffScreenBuffer that contains the Gamerender
			// to the screen using a rectshape.
			// While in 'ShowEditorUI'-mode this will instead happen through the 'GameViewDockable'.
			Vector2f adjustedSize = GetSizeThatRespectsAspectRatio(static_cast<int>(myWindowRect.myExtents.x), static_cast<int>(myWindowRect.myExtents.y));

			sf::RectangleShape rect;
			rect.setTexture(&myRenderer->GetOffscreenBuffer()->getTexture());
			rect.setSize({ adjustedSize.x, adjustedSize.y });

			myRenderWindow->draw(rect);
		}

		myRenderWindow->display();

		if (myScreenshotRequested)
		{
			myScreenshotRequested = false;
			SaveScreenshot();
		}

		myRenderWindow->clear();
	}

	void Window::SaveScreenshot()
	{
		sf::Texture texture;
		bool resizeSuccess = texture.resize(myRenderWindow->getSize());
		FW_ASSERT(resizeSuccess);
		texture.update(*myRenderWindow);

		sf::Image image = texture.copyToImage();

		FW_String screenshotPath = "data/debug/screenshot.png";
		FW_FileSystem::CreateFolderIfNecessary(screenshotPath);

		FW_String absoluteScreenshotPath;
		FW_FileSystem::GetAbsoluteFilePath(screenshotPath, absoluteScreenshotPath);

		FW_String absolutePreviousPath;
		FW_FileSystem::GetAbsoluteFilePath("data/debug/screenshot_previous.png", absolutePreviousPath);

		FW_FileSystem::FileInfo existingScreenshotInfo;
		if (FW_FileSystem::GetFileInfo(absoluteScreenshotPath, existingScreenshotInfo))
		{
			if (!FW_FileSystem::RenameFile(absoluteScreenshotPath, absolutePreviousPath))
				SLUSH_ERROR("SaveScreenshot: failed to rotate '%s' to '%s'", absoluteScreenshotPath.GetBuffer(), absolutePreviousPath.GetBuffer());
		}

		bool saveSuccess = image.saveToFile(absoluteScreenshotPath.GetBuffer());
		FW_ASSERT(saveSuccess);
	}

	void Window::Close()
	{
		if (!myAppLayout || !myAppLayout->HasUnsavedChanges())
		{
			myShouldBeOpen = false;
			return;
		}

		myCloseRequested = true;
	}

	void Window::UpdatePendingClose()
	{
		if (!myCloseRequested)
			return;

		if (!myAppLayout)
		{
			myShouldBeOpen = false;
			myCloseRequested = false;
			return;
		}

		// Window::Present() only updates the app layout inside 'if (myShowEditorUI)' - force it on so a
		// quit initiated from the game view still gets to draw and take input on the confirmation popup.
		myShowEditorUI = true;

		switch (myAppLayout->RequestClose())
		{
		case IAppLayout::CloseRequestResult::Resolved:
			myShouldBeOpen = false;
			myCloseRequested = false;
			break;
		case IAppLayout::CloseRequestResult::Cancelled:
			myCloseRequested = false;
			break;
		case IAppLayout::CloseRequestResult::StillPending:
			break;
		}
	}

	void Window::SetAppLayout(IAppLayout* aLayout)
	{
		SaveAppLayoutConfig();

		FW_SAFE_DELETE(myAppLayout);

		myAppLayout = aLayout;

		LoadAppLayoutConfig();
	}

	void Window::UpdateAppLayout()
	{
		if (myAppLayout)
			myAppLayout->Update();
	}

	void Window::RenderAppLayout()
	{
		if (myAppLayout)
			myAppLayout->Render();
	}

	void Window::SaveAppLayoutConfig()
	{
		if (!myAppLayout)
			return;

		FW_String path;
		FW_String settingName = "ImGUILayouts/";
		settingName += myAppLayout->GetName();
		settingName += ".ini";
		FW_FileSystem::GetAbsoluteFilePath(settingName, path);
		ImGui::SaveIniSettingsToDisk(path.GetBuffer());
	}

	void Window::LoadAppLayoutConfig()
	{
		if (!myAppLayout)
			return;

		FW_String path;
		FW_String settingName = "ImGUILayouts/";
		settingName += myAppLayout->GetName();
		settingName += ".ini";
		FW_FileSystem::GetAbsoluteFilePath(settingName, path);
		ImGui::LoadIniSettingsFromDisk(path.GetBuffer());
	}

	Vector2f Window::GetSizeThatRespectsAspectRatio(int aWidth, int aHeight) const
	{
		int adjustedWidth = static_cast<int>(aHeight * myAspectRatio);
		int adjustedHeight = static_cast<int>(aWidth * (1.f / myAspectRatio));

		if (aHeight >= adjustedHeight)
			adjustedWidth = aWidth;
		else
			adjustedHeight = aHeight;

		return { float(adjustedWidth), float(adjustedHeight) };
	}
}