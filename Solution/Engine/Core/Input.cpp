#include "stdafx.h"
#include "Input.h"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace Slush
{
	sf::Keyboard::Key GetSFMLKey(Input::KeyCode aEngineKeyCode)
	{
		switch (aEngineKeyCode)
		{
		case Input::KeyCode::LEFT: return sf::Keyboard::Key::Left;
		case Input::KeyCode::RIGHT: return sf::Keyboard::Key::Right;
		case Input::KeyCode::UP: return sf::Keyboard::Key::Up;
		case Input::KeyCode::DOWN: return sf::Keyboard::Key::Down;
		case Input::KeyCode::Q: return sf::Keyboard::Key::Q;
		case Input::KeyCode::W: return sf::Keyboard::Key::W;
		case Input::KeyCode::E: return sf::Keyboard::Key::E;
		case Input::KeyCode::A: return sf::Keyboard::Key::A;
		case Input::KeyCode::S: return sf::Keyboard::Key::S;
		case Input::KeyCode::D: return sf::Keyboard::Key::D;
		case Input::KeyCode::Z: return sf::Keyboard::Key::Z;
		case Input::KeyCode::X: return sf::Keyboard::Key::X;
		case Input::KeyCode::V: return sf::Keyboard::Key::V;
		case Input::KeyCode::SPACE: return sf::Keyboard::Key::Space;
		case Input::KeyCode::ESC: return sf::Keyboard::Key::Escape;
		case Input::KeyCode::DEL: return sf::Keyboard::Key::Delete;
		case Input::KeyCode::HYPHEN: return sf::Keyboard::Key::Hyphen;
		case Input::KeyCode::_1: return sf::Keyboard::Key::Num1;
		case Input::KeyCode::_2: return sf::Keyboard::Key::Num2;
		case Input::KeyCode::_3: return sf::Keyboard::Key::Num3;
		case Input::KeyCode::_4: return sf::Keyboard::Key::Num4;
		case Input::KeyCode::_5: return sf::Keyboard::Key::Num5;
		case Input::KeyCode::_6: return sf::Keyboard::Key::Num6;
		case Input::KeyCode::_7: return sf::Keyboard::Key::Num7;
		case Input::KeyCode::_8: return sf::Keyboard::Key::Num8;
		case Input::KeyCode::_9: return sf::Keyboard::Key::Num9;
		case Input::KeyCode::_0: return sf::Keyboard::Key::Num0;
		case Input::KeyCode::_F1: return sf::Keyboard::Key::F1;
		case Input::KeyCode::_F2: return sf::Keyboard::Key::F2;
		case Input::KeyCode::_F3: return sf::Keyboard::Key::F3;
		case Input::KeyCode::_F4: return sf::Keyboard::Key::F4;
		case Input::KeyCode::_F5: return sf::Keyboard::Key::F5;
		case Input::KeyCode::_F6: return sf::Keyboard::Key::F6;
		case Input::KeyCode::_F7: return sf::Keyboard::Key::F7;
		case Input::KeyCode::_F8: return sf::Keyboard::Key::F8;
		case Input::KeyCode::_F9: return sf::Keyboard::Key::F9;
		case Input::KeyCode::_F10: return sf::Keyboard::Key::F10;
		case Input::KeyCode::_F11: return sf::Keyboard::Key::F11;
		case Input::KeyCode::_F12: return sf::Keyboard::Key::F12;
		default:
			break;
		}

		return sf::Keyboard::Key::Escape;
	}
	sf::Mouse::Button GetSFMLMouseButton(Input::MouseButton aEngineMouseButton)
	{
		switch (aEngineMouseButton)
		{
		case Input::MouseButton::LEFTMB: return sf::Mouse::Button::Left;
		case Input::MouseButton::MIDDLEMB: return sf::Mouse::Button::Middle;
		case Input::MouseButton::RIGHTMB: return sf::Mouse::Button::Right;
		default:
			break;
		}

		return sf::Mouse::Button::Left;
	}

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
			sf::Mouse::Button sfmlKey = GetSFMLMouseButton(MouseButton(i));

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
			sf::Keyboard::Key sfmlKey = GetSFMLKey(KeyCode(i));

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
}