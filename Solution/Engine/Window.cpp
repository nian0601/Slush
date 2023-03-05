#include "stdafx.h"

#include "Window.h"

#include <SFML/Graphics.hpp>

namespace Slush
{
	Window::Window(int aWidth, int aHeight)
	{
		myRenderWindow = new sf::RenderWindow(sf::VideoMode(aWidth, aHeight), "Slush Engine");
	}

	Window::~Window()
	{
		FW_SAFE_DELETE(myRenderWindow);
	}

	bool Window::PumpEvents()
	{
		sf::Event event;
		while (myRenderWindow->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				return false;
		}

		return true;
	}
}