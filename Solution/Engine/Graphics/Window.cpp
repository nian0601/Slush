#include "stdafx.h"

#include "Graphics/Window.h"
#include "Graphics/Renderer.h"
#include "Core/Log.h"
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
		imguiIO.IniFilename = NULL;

		

		// Having this enabled makes it tricky/impossible to have Keyboard-input for the gameview :/
		// An option could be to have this be togglable somehow maybe?

		imguiIO.ConfigWindowsMoveFromTitleBarOnly = true;
		SLUSH_INFO("Window Created");
	}

	Window::~Window()
	{
		SaveAppLayoutConfig();

		myRegisteredLayouts.DeleteAll();

		FW_SAFE_DELETE(myRenderer);
		FW_SAFE_DELETE(myRenderWindow);
	}

	void Window::Hide()
	{
		myRenderWindow->setVisible(false);
	}

	void Window::PumpEvents()
	{
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
				sf::FloatRect visibleArea({ 0.f, 0.f }, { static_cast<float>(resized->size.x), static_cast<float>(resized->size.y) });
				myRenderWindow->setView(sf::View(visibleArea));
			}
		}
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

	void Window::BuildEditorChrome()
	{
		if (!myShowEditorUI)
			return;

		if (ImGui::BeginMainMenuBar())
		{
			if (myAppLayout)
				ImGui::Text("[ %s ]", myAppLayout->GetName().GetBuffer());

			if (ImGui::BeginMenu("Layouts"))
			{
				for (IAppLayoutFactory* factory : myRegisteredLayouts)
				{
					if (ImGui::Selectable(factory->GetMenuLabel()))
						RequestLayoutSwitch(*factory);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("ImGUI"))
			{
				ImGui::Checkbox("Show Demo", &myDisplayImGUIDemo);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		ImGui::DockSpaceOverViewport();

		if (myDisplayImGUIDemo)
			ImGui::ShowDemoWindow(&myDisplayImGUIDemo);

		if (myAppLayout)
			myAppLayout->BuildUI();
	}

	void Window::Composite()
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

	void Window::Present()
	{
		ImGui::SFML::Render(*myRenderWindow);

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
			ConfirmClose("no unsaved changes");
			return;
		}

		myPendingTransition = PendingTransition::Close;
	}

	void Window::ConfirmClose(const char* aReason)
	{
		SLUSH_INFO("[Window] Closing gracefully (%s)", aReason);
		myShouldBeOpen = false;
	}

	void Window::RegisterLayout(IAppLayoutFactory* aFactory)
	{
		FW_ASSERT(aFactory, "RegisterLayout: aFactory is null");
		myRegisteredLayouts.Add(aFactory);
	}

	void Window::RequestLayoutSwitch(IAppLayoutFactory& aFactory)
	{
		if (!myAppLayout || !myAppLayout->HasUnsavedChanges())
		{
			SetAppLayout(aFactory.CreateLayout());
			return;
		}

		myPendingTransition = PendingTransition::SwitchLayout;
		myPendingLayoutFactory = &aFactory;
	}

	void Window::UpdatePendingClose()
	{
		if (myPendingTransition == PendingTransition::None)
			return;

		if (!myAppLayout)
		{
			ResolvePendingTransition("no app layout");
			return;
		}

		// The close-confirmation modal is built in the editor-only BuildUI() half, so the editor must be
		// visible for the user to see and click it.
		myShowEditorUI = true;

		switch (myAppLayout->RequestClose())
		{
		case IAppLayout::CloseRequestResult::Resolved:
			ResolvePendingTransition("unsaved changes resolved");
			break;
		case IAppLayout::CloseRequestResult::Cancelled:
			myPendingTransition = PendingTransition::None;
			myPendingLayoutFactory = nullptr;
			break;
		case IAppLayout::CloseRequestResult::StillPending:
			break;
		}
	}

	void Window::ResolvePendingTransition(const char* aCloseReason)
	{
		const PendingTransition transition = myPendingTransition;
		IAppLayoutFactory* factory = myPendingLayoutFactory;

		myPendingTransition = PendingTransition::None;
		myPendingLayoutFactory = nullptr;

		switch (transition)
		{
		case PendingTransition::Close:
			ConfirmClose(aCloseReason);
			break;
		case PendingTransition::SwitchLayout:
			FW_ASSERT(factory, "Pending layout switch resolved with no target factory");
			SetAppLayout(factory->CreateLayout());
			break;
		case PendingTransition::None:
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

		FW_String folderPath;
		FW_FileSystem::GetAbsoluteFilePath("ImGUILayouts", folderPath);
		FW_FileSystem::CreateFolder(folderPath);

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