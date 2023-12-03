#include "stdafx.h"

#include "Graphics/Window.h"
#include "Core/Log.h"
#include "Core/Engine.h"
#include "Core/Time.h"
#include "Core/Dockables/Dockable.h"


#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

namespace Slush
{
	Window::Window(int aWidth, int aHeight)
		: myAspectRatio(16.f/9.f)
	{
		myWindowRect = MakeRectFromTopLeft<float>({ 0.f, 0.f }, { static_cast<float>(aWidth), static_cast<float>(aHeight) });
		myGameViewRect = MakeRectFromTopLeft<float>({ 0.f, 0.f }, GetSizeThatRespectsAspectRatio(aWidth, aHeight));

		myRenderWindow = new sf::RenderWindow(sf::VideoMode(aWidth, aHeight), "Slush Engine");
		myOffscreenBuffer = new sf::RenderTexture();
		myOffscreenBuffer->create(1920, 1080);

		myActiveRenderTarget = myRenderWindow;

		ImGui::SFML::Init(*myRenderWindow);
		
		ImGuiIO& imguiIO = ImGui::GetIO();
		imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// Having this enabled makes it tricky/impossible to have Keyboard-input for the gameview :/
		// An option could be to have this be togglable somehow maybe?
		//imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		imguiIO.ConfigWindowsMoveFromTitleBarOnly = true;

		SLUSH_INFO("Window Created");
	}

	Window::~Window()
	{
		FW_SAFE_DELETE(myOffscreenBuffer);
		FW_SAFE_DELETE(myRenderWindow);
		myDockables.DeleteAll();
	}

	bool Window::PumpEvents()
	{
		if (!myShouldBeOpen)
			return false;

		sf::Event event;
		while (myRenderWindow->pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed)
			{
				return false;
			}
			else if (event.type == sf::Event::Resized)
			{
				SetRectSize(myWindowRect, { static_cast<float>(event.size.width), static_cast<float>(event.size.height) });
			}
		}

		if (myShowEditorUI)
			ImGui::SFML::Update(*myRenderWindow , Time::GetDelta());

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

		ImTextureID textureID = myOffscreenBuffer->getTexture().getNativeHandle();
		ImGui::Image(textureID, { myGameViewRect.myExtents.x, myGameViewRect.myExtents.y }, { 0, 1 }, { 1, 0 });
	}

	void Window::Present()
	{
		if (myShowEditorUI)
		{
			ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
			ImGui::ShowDemoWindow();

			for (Dockable* dockable : myDockables)
				dockable->Update();

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
			rect.setTexture(&myOffscreenBuffer->getTexture());
			rect.setSize({ adjustedSize.x, adjustedSize.y });

			myRenderWindow->draw(rect);
		}

		myRenderWindow->display();
		myRenderWindow->clear();
	}

	void Window::StartOffscreenBuffer()
	{
		myActiveRenderTarget = myOffscreenBuffer;
		myOffscreenBuffer->clear(sf::Color(128, 180, 200));
	}

	void Window::EndOffscreenBuffer()
	{
		myOffscreenBuffer->display();
		myActiveRenderTarget = myRenderWindow;
	}

	void Window::AddDockable(Dockable* aDockable)
	{
		aDockable->myDockableID = myNextDockableID++;
		myDockables.Add(aDockable);
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