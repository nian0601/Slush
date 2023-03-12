#include "stdafx.h"

#include "Graphics/Window.h"
#include "Core/Log.h"
#include "Core/Engine.h"


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

		SLUSH_INFO("Window Created");

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
		if (ImGui::Begin("Log"))
		{
			const FW_GrowingArray<Logger::LogEntry>& entries = Engine::GetInstance().GetLogger().GetEntries();
			ImGui::BeginGroup();
			for (const Logger::LogEntry& entry : entries)
			{
				Vector3f color = Logger::GetSeverityColorVec(entry.mySeverity);
				ImVec4 imColor;
				imColor.x = color.x;
				imColor.y = color.y;
				imColor.z = color.z;
				imColor.w = 1.f;
				ImGui::TextColored(imColor, entry.myMessage.GetBuffer());
			}
			ImGui::EndGroup();
		}
		ImGui::End();

		ImGui::SFML::Render(*myRenderWindow);
	}

	void Window::RenderOffscreenBufferToImGUI()
	{
		float width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
		float height = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;

		ImTextureID textureID = myOffscreenBuffer->getTexture().getNativeHandle();
		ImGui::Image(textureID, { width, height }, { 0, 1 }, { 1, 0 });
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