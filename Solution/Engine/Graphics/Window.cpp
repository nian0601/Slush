#include "stdafx.h"

#include "Graphics/Window.h"
#include "Core/Log.h"
#include "Core/Engine.h"
#include "Core/Time.h"
#include "Core/Dockables/Dockable.h"


#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include <FW_FileSystem.h>
#include <SFML/Graphics/VertexArray.hpp>

namespace Slush
{
	sf::Color GetSFMLColor(int aHexColor)
	{
		return{
			unsigned char((aHexColor >> 16) & 255),
			unsigned char((aHexColor >> 8) & 255),
			unsigned char((aHexColor >> 0) & 255),
			unsigned char((aHexColor >> 24) & 255)
		};
	}

	Window::Window(unsigned int aWidth, unsigned int aHeight)
		: myAspectRatio(16.f/9.f)
	{
		myWindowRect = MakeRectFromTopLeft<float>({ 0.f, 0.f }, { static_cast<float>(aWidth), static_cast<float>(aHeight) });
		myGameViewRect = MakeRectFromTopLeft<float>({ 0.f, 0.f }, GetSizeThatRespectsAspectRatio(aWidth, aHeight));

		myRenderWindow = new sf::RenderWindow(sf::VideoMode({ aWidth, aHeight }), "Slush Engine");
		myOffscreenBuffer = new sf::RenderTexture({ 1920, 1080 });

		myCircleShape = new sf::CircleShape();
		myRectShape = new sf::RectangleShape();

		myActiveRenderTarget = myRenderWindow;

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

		FW_SAFE_DELETE(myOffscreenBuffer);
		FW_SAFE_DELETE(myRenderWindow);
		FW_SAFE_DELETE(myCircleShape);
		FW_SAFE_DELETE(myRectShape);
		myDockables.DeleteAll();
	}

	bool Window::PumpEvents()
	{
		if (!myShouldBeOpen)
			return false;

		//sf::Event event;
		//while (myRenderWindow->pollEvent(event))
		//{
		//	ImGui::SFML::ProcessEvent(*myRenderWindow, event);
		//
		//	if (event.type == sf::Event::Closed)
		//	{
		//		return false;
		//	}
		//	else if (event.type == sf::Event::Resized)
		//	{
		//		SetRectSize(myWindowRect, { static_cast<float>(event.size.width), static_cast<float>(event.size.height) });
		//		//myGameViewRect = MakeRectFromTopLeft<float>({ 0.f, 0.f }, GetSizeThatRespectsAspectRatio(aWidth, aHeight));
		//		//myRenderWindow->setSize({ event.size.width, event.size.height });
		//		sf::FloatRect visibleArea(0.f, 0.f, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
		//		myRenderWindow->setView(sf::View(visibleArea));
		//	}
		//}

		while (const std::optional event = myRenderWindow->pollEvent())
		{
			ImGui::SFML::ProcessEvent(*myRenderWindow, *event);

			if (event->is<sf::Event::Closed>())
			{
				return false;
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

		ImTextureID textureID = myOffscreenBuffer->getTexture().getNativeHandle();
		ImGui::Image(textureID, { myGameViewRect.myExtents.x, myGameViewRect.myExtents.y }, { 0, 1 }, { 1, 0 });
	}

	void Window::Present()
	{
		if (myShowEditorUI)
		{
			if (ImGui::BeginMainMenuBar())
			{
				ImGui::Text("[ %s ]", myAppLayoutName);
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

	void Window::DeleteAllDockables()
	{
		myDockables.DeleteAll();
		myNextDockableID = 0;
	}

	void Window::SetAppLayout(const char* aName)
	{
		SaveAppLayoutConfig();

		myDockables.DeleteAll();
		myNextDockableID = 0;

		myAppLayoutName = aName;

		LoadAppLayoutConfig();
	}

	void Window::RenderLine(const Vector2i& aStart, const Vector2i& aEnd, int aColor)
	{
		sf::VertexArray line(sf::PrimitiveType::Lines, 2);
		line[0].position = { float(aStart.x), float(aStart.y) };
		line[0].color = GetSFMLColor(aColor);

		line[1].position = { float(aEnd.x), float(aEnd.y) };
		line[1].color = GetSFMLColor(aColor);

		GetActiveRenderTarget()->draw(line);
	}

	void Window::RenderLine(const Vector2f& aStart, const Vector2f& aEnd, int aColor)
	{
		sf::VertexArray line(sf::PrimitiveType::Lines, 2);
		line[0].position = { aStart.x, aStart.y };
		line[0].color = GetSFMLColor(aColor);

		line[1].position = { aEnd.x, aEnd.y };
		line[1].color = GetSFMLColor(aColor);

		GetActiveRenderTarget()->draw(line);
	}

	void Window::RenderRect(const Rectf& aRect, int aColor, float aRotationInRadians)
	{
		myRectShape->setOrigin({ aRect.myExtents.x * 0.5f, aRect.myExtents.y * 0.5f });
		myRectShape->setPosition({ aRect.myCenterPos.x, aRect.myCenterPos.y });
		myRectShape->setRotation(sf::radians(aRotationInRadians));
		
		sf::Vector2f oldSize = myRectShape->getSize();
		if (oldSize.x != aRect.myExtents.x || oldSize.y != aRect.myExtents.y)
		{
			myRectShape->setSize({ aRect.myExtents.x, aRect.myExtents.y });
		}

		myRectShape->setFillColor(GetSFMLColor(aColor));
		myRectShape->setTexture(nullptr);

		GetActiveRenderTarget()->draw(*myRectShape);
	}

	void Window::RenderCircle(const Vector2f& aCenter, float aRadius, int aColor)
	{
		myCircleShape->setPosition({ aCenter.x - aRadius, aCenter.y - aRadius });

		if (myCircleShape->getRadius() != aRadius)
			myCircleShape->setRadius(aRadius);

		myCircleShape->setFillColor(GetSFMLColor(aColor));
		GetActiveRenderTarget()->draw(*myCircleShape);
	}

	void Window::SaveAppLayoutConfig()
	{
		FW_String path;
		FW_String settingName = "ImGUILayouts/";
		settingName += myAppLayoutName;
		settingName += ".ini";
		FW_FileSystem::GetAbsoluteFilePath(settingName, path);
		ImGui::SaveIniSettingsToDisk(path.GetBuffer());
	}

	void Window::LoadAppLayoutConfig()
	{
		FW_String path;
		FW_String settingName = "ImGUILayouts/";
		settingName += myAppLayoutName;
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