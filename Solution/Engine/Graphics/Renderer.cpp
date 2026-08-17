#include "stdafx.h"

#include "Graphics/Renderer.h"
#include "Graphics/SFMLHelpers.h"
#include "Core/Time.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Texture.hpp>

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

		myFadeData.myFadeTexture = new sf::Texture();
		bool resizeSuccess = myFadeData.myFadeTexture->resize({ 1920, 1080 });
		FW_ASSERT(resizeSuccess);
	}

	Renderer::~Renderer()
	{
		FW_SAFE_DELETE(myOffscreenBuffer);
		FW_SAFE_DELETE(myCircleShape);
		FW_SAFE_DELETE(myRectShape);
		FW_SAFE_DELETE(myFadeData.myFadeTexture);
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

	Vector2f Renderer::GetOffscreenBufferSize() const
	{
		sf::Vector2u size = myOffscreenBuffer->getSize();
		return { static_cast<float>(size.x), static_cast<float>(size.y) };
	}

	Renderer::TargetQueue& Renderer::GetOrCreateTargetQueue(sf::RenderTarget* aTarget)
	{
		for (TargetQueue& queue : myTargetQueues)
		{
			if (queue.myTarget == aTarget)
				return queue;
		}

		TargetQueue& newQueue = myTargetQueues.Add();
		newQueue.myTarget = aTarget;
		return newQueue;
	}

	void Renderer::RenderLine(const Vector2i& aStart, const Vector2i& aEnd, int aColor)
	{
		RenderCommand& command = GetOrCreateTargetQueue(GetActiveRenderTarget()).myCommands.Add();
		command.myType = RenderCommand::Type::Line;
		command.myPoint1 = { float(aStart.x), float(aStart.y) };
		command.myPoint2 = { float(aEnd.x), float(aEnd.y) };
		command.myColor = aColor;
	}

	void Renderer::RenderLine(const Vector2f& aStart, const Vector2f& aEnd, int aColor)
	{
		RenderCommand& command = GetOrCreateTargetQueue(GetActiveRenderTarget()).myCommands.Add();
		command.myType = RenderCommand::Type::Line;
		command.myPoint1 = aStart;
		command.myPoint2 = aEnd;
		command.myColor = aColor;
	}

	void Renderer::RenderTriangle(const Vector2f& aV1, const Vector2f& aV2, const Vector2f& aV3, int aColor /*= 0xFFFFFFFF*/)
	{
		RenderCommand& command = GetOrCreateTargetQueue(GetActiveRenderTarget()).myCommands.Add();
		command.myType = RenderCommand::Type::Triangle;
		command.myPoint1 = aV1;
		command.myPoint2 = aV2;
		command.myPoint3 = aV3;
		command.myColor = aColor;
	}

	void Renderer::RenderRect(const Rectf& aRect, int aColor, float aRotationInRadians)
	{
		RenderCommand& command = GetOrCreateTargetQueue(GetActiveRenderTarget()).myCommands.Add();
		command.myType = RenderCommand::Type::Rect;
		command.myRect = aRect;
		command.myColor = aColor;
		command.myRotationInRadians = aRotationInRadians;
	}

	void Renderer::RenderCircle(const Vector2f& aCenter, float aRadius, int aColor)
	{
		RenderCommand& command = GetOrCreateTargetQueue(GetActiveRenderTarget()).myCommands.Add();
		command.myType = RenderCommand::Type::Circle;
		command.myCenter = aCenter;
		command.myRadius = aRadius;
		command.myColor = aColor;
	}

	void Renderer::ProcessRenderQueue()
	{
		for (TargetQueue& queue : myTargetQueues)
		{
			for (RenderCommand& command : queue.myCommands)
			{
				switch (command.myType)
				{
				case RenderCommand::Type::Line:
				{
					sf::VertexArray line(sf::PrimitiveType::Lines, 2);
					line[0].position = { command.myPoint1.x, command.myPoint1.y };
					line[0].color = GetSFMLColor(command.myColor);

					line[1].position = { command.myPoint2.x, command.myPoint2.y };
					line[1].color = GetSFMLColor(command.myColor);

					queue.myTarget->draw(line);
					break;
				}
				case RenderCommand::Type::Triangle:
				{
					sf::VertexArray triangle(sf::PrimitiveType::Triangles, 3);
					triangle[0].position = { command.myPoint1.x, command.myPoint1.y };
					triangle[0].color = GetSFMLColor(command.myColor);

					triangle[1].position = { command.myPoint2.x, command.myPoint2.y };
					triangle[1].color = GetSFMLColor(command.myColor);

					triangle[2].position = { command.myPoint3.x, command.myPoint3.y };
					triangle[2].color = GetSFMLColor(command.myColor);

					queue.myTarget->draw(triangle);
					break;
				}
				case RenderCommand::Type::Rect:
				{
					myRectShape->setOrigin({ command.myRect.myExtents.x * 0.5f, command.myRect.myExtents.y * 0.5f });
					myRectShape->setPosition({ command.myRect.myCenterPos.x, command.myRect.myCenterPos.y });
					myRectShape->setRotation(sf::radians(command.myRotationInRadians));

					sf::Vector2f oldSize = myRectShape->getSize();
					if (oldSize.x != command.myRect.myExtents.x || oldSize.y != command.myRect.myExtents.y)
					{
						myRectShape->setSize({ command.myRect.myExtents.x, command.myRect.myExtents.y });
					}

					myRectShape->setFillColor(GetSFMLColor(command.myColor));
					myRectShape->setTexture(nullptr);

					queue.myTarget->draw(*myRectShape);
					break;
				}
				case RenderCommand::Type::Circle:
				{
					myCircleShape->setPosition({ command.myCenter.x - command.myRadius, command.myCenter.y - command.myRadius });

					if (myCircleShape->getRadius() != command.myRadius)
						myCircleShape->setRadius(command.myRadius);

					myCircleShape->setFillColor(GetSFMLColor(command.myColor));
					queue.myTarget->draw(*myCircleShape);
					break;
				}
				case RenderCommand::Type::Sprite:
				case RenderCommand::Type::Text:
				default:
					break;
				}
			}

			queue.myCommands.RemoveAll();
		}
	}

	void Renderer::StartFade(float aDuration)
	{
		myFadeData.myIsFading = true;
		myFadeData.myRemainingTime = aDuration;
		myFadeData.myTotalTime = aDuration;
	}

	void Renderer::RenderFade()
	{
		if (myFadeData.myIsFading)
		{
			myFadeData.myRemainingTime -= Time::GetDelta();
			if (myFadeData.myRemainingTime <= 0.f)
				myFadeData.myIsFading = false;
		}

		if (myFadeData.myIsFading)
		{
			float alpha = FW_Max(0.f, myFadeData.myRemainingTime / myFadeData.myTotalTime);
			sf::RectangleShape rect;
			rect.setTexture(myFadeData.myFadeTexture);
			rect.setSize({ 1920.f, 1080.f });
			rect.setFillColor(SFMLHelpers::GetColor(FW_Float_To_ARGB(alpha, 1.f, 1.f, 1.f)));
			myOffscreenBuffer->draw(rect);
		}

		myOffscreenBuffer->display();

		if (!myFadeData.myIsFading)
			myFadeData.myFadeTexture->update(myOffscreenBuffer->getTexture());
	}
}
