#pragma once
#include "UIWidget.h"

namespace Slush
{
	class RectSprite;
	class UIRect : public UIWidget
	{
	public:
		UIRect();
		~UIRect();

		void SetSize(const Vector2i& aSize);
		void SetPosition(const Vector2i& aPosition);
		void SetColors(int aColor, int aOutlineColor);

		void Render() override;

	private:
		RectSprite* mySprite = nullptr;
		int myColor = 0xFF333333;
		int myOutlineColor= 0xFF555555;
		Rectf myRect;
	};
}
