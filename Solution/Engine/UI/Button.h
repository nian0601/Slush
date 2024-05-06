#pragma once

namespace Slush
{
	class RectSprite;
	class Input;
	class Button
	{
	public:
		Button();
		~Button();

		void Update(const Input& aInput);
		void Render();

		bool WasPressed() const;


	private:
		enum State
		{
			INACTIVE,
			HOVERED,
			MOUSE_DOWN,
		};

		State myPreviousState = INACTIVE;
		State myState = INACTIVE;
		RectSprite* mySprite = nullptr;
		Rectf myRect;
	};
}
