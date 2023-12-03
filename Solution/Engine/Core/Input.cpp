#include "stdafx.h"
#include "Input.h"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace Slush
{

	Input::Input()
	{
		for (int i = 0; i < KeyCode::__COUNT; ++i)
			myKeyStates[i] = State::KEYSTATE_NONE;

		for (int i = 0; i < MouseButton::___COUNT; ++i)
			myMouseStates[i] = State::KEYSTATE_NONE;
	}

	void Input::UpdateMouse(sf::RenderWindow& aRenderWindow)
	{
		for (int i = 0; i < MouseButton::___COUNT; ++i)
			UpdateState(myMouseStates[i]);

		for (int i = 0; i < MouseButton::___COUNT; ++i)
		{
			sf::Mouse::Button sfmlKey = sf::Mouse::Button(GetSFMLMouseButton(MouseButton(i)));

			State prevState = myMouseStates[i];
			bool isCurrentlyPressed = sf::Mouse::isButtonPressed(sfmlKey);

			if (isCurrentlyPressed && prevState == KEYSTATE_NONE)
				myMouseStates[i] = KEYSTATE_PRESSED;
			else if (!isCurrentlyPressed && prevState == KEYSTATE_DOWN)
				myMouseStates[i] = KEYSTATE_RELEASED;
		}

		sf::Vector2i sfmlPosition = sf::Mouse::getPosition(aRenderWindow);
		myMousePosition.x = sfmlPosition.x;
		myMousePosition.y = sfmlPosition.y;

		myMousePositionf.x = static_cast<float>(sfmlPosition.x);
		myMousePositionf.y = static_cast<float>(sfmlPosition.y);
	}

	void Input::UpdateKeyboard()
	{
		for (int i = 0; i < KeyCode::__COUNT; ++i)
			UpdateState(myKeyStates[i]);

		for (int i = 0; i < KeyCode::__COUNT; ++i)
		{
			sf::Keyboard::Key sfmlKey = sf::Keyboard::Key(GetSFMLKey(KeyCode(i)));

			State prevState = myKeyStates[i];
			bool isCurrentlyPressed = sf::Keyboard::isKeyPressed(sfmlKey);

			if (isCurrentlyPressed && prevState == KEYSTATE_NONE)
				myKeyStates[i] = KEYSTATE_PRESSED;
			else if (!isCurrentlyPressed && prevState == KEYSTATE_DOWN)
				myKeyStates[i] = KEYSTATE_RELEASED;
		}
	}

	void Input::RemapMousePosition(const Rectf& aWindowRect, const Rectf& aGameViewRect)
	{
		float gameViewX = FW_UnLerp(aGameViewRect.myTopLeft.x, aGameViewRect.myBottomRight.x, myMousePositionf.x);
		float gameViewY = FW_UnLerp(aGameViewRect.myTopLeft.y, aGameViewRect.myBottomRight.y, myMousePositionf.y);

		myMousePositionf.x = FW_Lerp(aWindowRect.myTopLeft.x, aWindowRect.myBottomRight.x, gameViewX);;
		myMousePositionf.y = FW_Lerp(aWindowRect.myTopLeft.y, aWindowRect.myBottomRight.y, gameViewY);;

		myMousePosition.x = static_cast<int>(myMousePositionf.x + 0.5f);
		myMousePosition.y = static_cast<int>(myMousePositionf.y + 0.5f);
	}

	void Input::UpdateState(State& aState)
	{
		switch (aState)
		{
		case KEYSTATE_PRESSED:
			aState = KEYSTATE_DOWN;
			break;
		case KEYSTATE_RELEASED:
			aState = KEYSTATE_NONE;
			break;
		case KEYSTATE_DOWN:
		case KEYSTATE_NONE:
			// Dont do anything
			break;
		}
	}

	int Input::GetSFMLKey(KeyCode aEngineKeyCode)
	{
		switch (aEngineKeyCode)
		{
		case LEFT: return sf::Keyboard::Left;
		case RIGHT: return sf::Keyboard::Right;
		case UP: return sf::Keyboard::Up;
		case DOWN: return sf::Keyboard::Down;
		case W: return sf::Keyboard::W;
		case S: return sf::Keyboard::S;
		case A: return sf::Keyboard::A;
		case D: return sf::Keyboard::D;
		case Q: return sf::Keyboard::Q;
		case E: return sf::Keyboard::E;
		case SPACE: return sf::Keyboard::Space;
		case ESC: return sf::Keyboard::Escape;
		case DEL: return sf::Keyboard::Delete;
		case HYPHEN: return sf::Keyboard::Hyphen;
		case _1: return sf::Keyboard::Num1;
		case _2: return sf::Keyboard::Num2;
		case _3: return sf::Keyboard::Num3;
		case _4: return sf::Keyboard::Num4;
		case _5: return sf::Keyboard::Num5;
		case _6: return sf::Keyboard::Num6;
		case _7: return sf::Keyboard::Num7;
		case _8: return sf::Keyboard::Num8;
		case _9: return sf::Keyboard::Num9;
		case _0: return sf::Keyboard::Num0;
		case _F1: return sf::Keyboard::F1;
		case _F2: return sf::Keyboard::F2;
		case _F3: return sf::Keyboard::F3;
		case _F4: return sf::Keyboard::F4;
		case _F5: return sf::Keyboard::F5;
		case _F6: return sf::Keyboard::F6;
		case _F7: return sf::Keyboard::F7;
		case _F8: return sf::Keyboard::F8;
		case _F9: return sf::Keyboard::F9;
		case _F10: return sf::Keyboard::F10;
		case _F11: return sf::Keyboard::F11;
		case _F12: return sf::Keyboard::F12;
		default:
			break;
		}

		return sf::Keyboard::Escape;
	}

	int Input::GetSFMLMouseButton(MouseButton aEngineMouseButton)
	{
		switch (aEngineMouseButton)
		{
		case LEFTMB: return sf::Mouse::Left;
		case MIDDLEMB: return sf::Mouse::Middle;
		case RIGHTMB: return sf::Mouse::Right;
		default:
			break;
		}

		return sf::Mouse::Left;
	}
}