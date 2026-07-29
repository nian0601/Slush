#include "stdafx.h"

#include "Graphics/Renderer.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
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

	Renderer::Renderer(sf::RenderWindow* aRenderWindow)
		: myRenderWindow(aRenderWindow)
	{
		myOffscreenBuffer = new sf::RenderTexture({ 1920, 1080 });

		myCircleShape = new sf::CircleShape();
		myRectShape = new sf::RectangleShape();

		myActiveRenderTarget = myRenderWindow;
	}

	Renderer::~Renderer()
	{
		FW_SAFE_DELETE(myOffscreenBuffer);
		FW_SAFE_DELETE(myCircleShape);
		FW_SAFE_DELETE(myRectShape);
	}

	void Renderer::StartOffscreenBuffer()
	{
		myActiveRenderTarget = myOffscreenBuffer;
		myOffscreenBuffer->clear(sf::Color(128, 180, 200));
	}

	void Renderer::EndOffscreenBuffer()
	{
		myActiveRenderTarget = myRenderWindow;
	}

	void Renderer::RenderLine(const Vector2i& aStart, const Vector2i& aEnd, int aColor)
	{
		sf::VertexArray line(sf::PrimitiveType::Lines, 2);
		line[0].position = { float(aStart.x), float(aStart.y) };
		line[0].color = GetSFMLColor(aColor);

		line[1].position = { float(aEnd.x), float(aEnd.y) };
		line[1].color = GetSFMLColor(aColor);

		GetActiveRenderTarget()->draw(line);
	}

	void Renderer::RenderLine(const Vector2f& aStart, const Vector2f& aEnd, int aColor)
	{
		sf::VertexArray line(sf::PrimitiveType::Lines, 2);
		line[0].position = { aStart.x, aStart.y };
		line[0].color = GetSFMLColor(aColor);

		line[1].position = { aEnd.x, aEnd.y };
		line[1].color = GetSFMLColor(aColor);

		GetActiveRenderTarget()->draw(line);
	}

	void Renderer::RenderTriangle(const Vector2f& aV1, const Vector2f& aV2, const Vector2f& aV3, int aColor /*= 0xFFFFFFFF*/)
	{
		sf::VertexArray triangle(sf::PrimitiveType::Triangles, 3);
		triangle[0].position = { aV1.x, aV1.y };
		triangle[0].color = GetSFMLColor(aColor);

		triangle[1].position = { aV2.x, aV2.y };
		triangle[1].color = GetSFMLColor(aColor);

		triangle[2].position = { aV3.x, aV3.y };
		triangle[2].color = GetSFMLColor(aColor);

		GetActiveRenderTarget()->draw(triangle);
	}

	void Renderer::RenderRect(const Rectf& aRect, int aColor, float aRotationInRadians)
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

	void Renderer::RenderCircle(const Vector2f& aCenter, float aRadius, int aColor)
	{
		myCircleShape->setPosition({ aCenter.x - aRadius, aCenter.y - aRadius });

		if (myCircleShape->getRadius() != aRadius)
			myCircleShape->setRadius(aRadius);

		myCircleShape->setFillColor(GetSFMLColor(aColor));
		GetActiveRenderTarget()->draw(*myCircleShape);
	}
}
