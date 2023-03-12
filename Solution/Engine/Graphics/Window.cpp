#include "stdafx.h"

#include "Window.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

namespace Slush
{
	Window::Window(int aWidth, int aHeight)
	{
		myRenderWindow = new sf::RenderWindow(sf::VideoMode(aWidth, aHeight), "Slush Engine");
		myOffscreenBuffer = new sf::RenderTexture();
		myOffscreenBuffer->create(1920, 1080);

		myActiveRenderTarget = myRenderWindow;

		ImGui::SFML::Init(*myRenderWindow);
		
		ImGuiIO& imguiIO = ImGui::GetIO();
		imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		imguiIO.ConfigWindowsMoveFromTitleBarOnly = true;
	}

	Window::~Window()
	{
		FW_SAFE_DELETE(myOffscreenBuffer);
		FW_SAFE_DELETE(myRenderWindow);
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
				return false;
		}

		ImGui::SFML::Update(*myRenderWindow , 1.f / 60.f);

		return true;
	}

	void Window::RenderImGUI()
	{
		ImGui::SFML::Render(*myRenderWindow);
	}

	void Window::Present()
	{
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

}