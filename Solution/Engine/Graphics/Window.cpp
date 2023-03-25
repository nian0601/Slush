#include "stdafx.h"

#include "Graphics/Window.h"
#include "Core/Log.h"
#include "Core/Engine.h"
#include "Core/Time.h"


#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

namespace Slush
{
	Window::Window(int aWidth, int aHeight)
		: myWidth(aWidth)
		, myHeight(aHeight)
		, myAspectRatio(16.f/9.f)
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
			{
				return false;
			}
			else if (event.type == sf::Event::Resized)
			{
				myWidth = event.size.width;
				myHeight = event.size.height;
			}
		}

		ImGui::SFML::Update(*myRenderWindow , Time::GetDelta());

		return true;
	}

	void Window::RenderImGUI()
	{
		if (!myShouldRenderOffscreenBufferToScreen)
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
		}
		ImGui::SFML::Render(*myRenderWindow);
	}

	void Window::RenderOffscreenBufferToImGUI()
	{
		float width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
		float height = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;

		Vector2f adjustedSize = GetSizeThatRespectsAspectRatio(static_cast<int>(width), static_cast<int>(height));

		ImTextureID textureID = myOffscreenBuffer->getTexture().getNativeHandle();
		ImGui::Image(textureID, { adjustedSize.x, adjustedSize.y }, { 0, 1 }, { 1, 0 });
	}

	void Window::Present()
	{
		if (myShouldRenderOffscreenBufferToScreen)
		{
			Vector2f adjustedSize = GetSizeThatRespectsAspectRatio(myWidth, myHeight);

			sf::RectangleShape rect;
			rect.setTexture(&myOffscreenBuffer->getTexture());
			rect.setSize({ adjustedSize.x, adjustedSize.y });
			//rect.setOrigin({ adjustedSize.x * 0.5f, adjustedSize.y * 0.5f });

			//float x = (myWidth - adjustedSize.x) * 0.5f;
			//float y = (myHeight - adjustedSize.y) * 0.5f;
			//rect.setPosition({ x, y });

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