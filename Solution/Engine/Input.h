#pragma once
#include "FW_Vector2.h"

namespace sf
{
	class RenderWindow;
}

namespace Slush
{
	class Input
	{
	public:
		enum KeyCode
		{
			LEFT,
			RIGHT,
			UP,
			DOWN,
			W,
			S,
			A,
			D,
			Q,
			E,
			SPACE,
			ESC,
			DEL,
			_1,
			_2,
			_3,
			_4,
			_5,
			_6,
			_7,
			_8,
			_9,
			_0,
			_F1,
			_F2,
			_F3,
			_F4,
			_F5,
			_F6,
			_F7,
			_F8,
			_F9,
			_F10,
			_F11,
			_F12,
			__COUNT,
		};

		enum MouseButton
		{
			LEFTMB,
			MIDDLEMB,
			RIGHTMB,
			___COUNT
		};

		Input();

		void UpdateMouse(sf::RenderWindow& aRenderWindow);
		void UpdateKeyboard();

		bool WasKeyPressed(KeyCode aKey) const { return myKeyStates[aKey] == KEYSTATE_PRESSED; }
		bool WasKeyReleased(KeyCode aKey) const { return myKeyStates[aKey] == KEYSTATE_RELEASED; }
		bool IsKeyDown(KeyCode aKey)const { return myKeyStates[aKey] == KEYSTATE_DOWN; }

		bool WasMousePressed(MouseButton aMouseButton) const { return myMouseStates[aMouseButton] == KEYSTATE_PRESSED; }
		bool WasMouseReleased(MouseButton aMouseButton) const { return myMouseStates[aMouseButton] == KEYSTATE_RELEASED; }
		bool IsMouseDown(MouseButton aMouseButton) const { return myMouseStates[aMouseButton] == KEYSTATE_DOWN; }

		const Vector2i& GetMousePosition() const { return myMousePosition; }
		const Vector2f& GetMousePositionf() const { return myMousePositionf; }

	private:
		enum State
		{
			KEYSTATE_NONE,
			KEYSTATE_PRESSED,
			KEYSTATE_DOWN,
			KEYSTATE_RELEASED,
		};

		void UpdateState(State& aState);
		int GetSFMLKey(KeyCode aEngineKeyCode);
		int GetSFMLMouseButton(MouseButton aEngineMouseButton);

		State myKeyStates[KeyCode::__COUNT + 1];
		State myMouseStates[MouseButton::___COUNT];
		Vector2i myMousePosition;
		Vector2f myMousePositionf;
	};
}