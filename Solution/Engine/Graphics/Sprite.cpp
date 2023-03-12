#include "stdafx.h"

#include "Core/Engine.h"

#include "Graphics/Sprite.h"
#include "Graphics/Window.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

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

	Sprite::Sprite()
	{
		myShape = new sf::RectangleShape();

		float w = 150.f;
		float h = 150.f;
		myShape->setSize({ w, h });
		myShape->setOrigin({ w * 0.5f, h * 0.5f });
		myShape->setFillColor(GetSFMLColor(0xFF00FF88));

		SLUSH_INFO("Sprite created");
	}

	Sprite::~Sprite()
	{
		FW_SAFE_DELETE(myShape);
	}

	void Sprite::Render(float x, float y)
	{
		myShape->setPosition({ x, y });
		Engine::GetInstance().GetWindow().GetActiveRenderTarget()->draw(*myShape);

		//sf::RectangleShape& rect = SFML_Renderer::ourRectangleShape;
		//rect.setOrigin({ aRect.myExtents.x * 0.5f, aRect.myExtents.y * 0.5f });
		//rect.setPosition(GetCameraAdjustedPosition(aRect.myCenterPos));
		//rect.setRotation(FW_RadiansToDegrees(aRotationInRadians));
		//if (aRect.myExtents != SFML_Renderer::ourCurrentRectangleSize)
		//{
		//	SFML_Renderer::ourCurrentRectangleSize = aRect.myExtents;
		//	rect.setSize({ aRect.myExtents.x, aRect.myExtents.y });
		//}
		//
		//rect.setFillColor(SFML_Renderer::GetSFMLColor(aColor));
		//rect.setTexture(nullptr);
		//
		//SFML_Renderer::ourOffscreenBuffer->draw(rect);
	}

}