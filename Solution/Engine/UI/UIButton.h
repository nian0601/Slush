#pragma once
#include "UIWidget.h"

namespace Slush
{
	class RectSprite;
	class Input;
	class UIButton : public UIWidget
	{
	public:
		UIButton();
		~UIButton();

		void SetSize(const Vector2i& aSize);
		void SetPosition(const Vector2i& aPosition);
		void SetColors(int aColor, int aHoverColor, int aPressedColor);

		void Update(const Input& aInput) override;
		void Render() override;
		void Inactivate() override;

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
		int myColor = 0xFF333333;
		int myHoverColor = 0xFF555555;
		int myPressedColor = 0xFF777777;
		Rectf myRect;
	};
}
