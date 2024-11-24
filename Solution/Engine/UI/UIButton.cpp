#include "stdafx.h"

#include "UIButton.h"

#include <Core\Input.h>
#include <Graphics\RectSprite.h>
#include <Graphics\Text.h>

namespace Slush
{
	UIButton::UIButton()
	{
		mySprite = new RectSprite();
		mySprite->SetSize(300.f, 150.f);
		mySprite->SetFillColor(myColor);
		myRect = MakeRectFromCenter(Vector2f(500.f, 500.f), mySprite->GetSize());
	}

	UIButton::~UIButton()
	{
		FW_SAFE_DELETE(mySprite);
	}

	void UIButton::SetSize(const Vector2i& aSize)
	{
		mySprite->SetSize(static_cast<float>(aSize.x), static_cast<float>(aSize.y));
	}

	void UIButton::SetPosition(const Vector2i& aPosition)
	{
		myRect = MakeRectFromCenter(Vector2f(static_cast<float>(aPosition.x), static_cast<float>(aPosition.y)), mySprite->GetSize());
	}

	void UIButton::SetColors(int aColor, int aHoverColor, int aPressedColor)
	{
		myColor = aColor;
		myHoverColor = aHoverColor;
		myPressedColor = aPressedColor;
	}

	void UIButton::SetText(const char* aText, Font& aFont)
	{
		if (!myText)
		{
			myText = new Text();
			myText->SetFont(aFont);
			myText->SetHorizontalAlignment(Text::HorizontalAlignment::CENTER);
			myText->SetVerticalAlignment(Text::VerticalAlignment::CENTER);
			myText->SetCharacterSize(45);
		}

		myText->SetText(aText);
	}

	void UIButton::Update(const Input& aInput)
	{
		myPreviousState = myState;
		myState = INACTIVE;

		if (Contains(myRect, aInput.GetMousePositionf()))
		{
			myState = HOVERED;

			if (aInput.IsMouseDown(Input::LEFTMB))
				myState = MOUSE_DOWN;
		}

		switch (myState)
		{
		case Slush::UIButton::INACTIVE:
			mySprite->SetFillColor(myColor);
			break;
		case Slush::UIButton::HOVERED:
			mySprite->SetFillColor(myHoverColor);
			break;
		case Slush::UIButton::MOUSE_DOWN:
			mySprite->SetFillColor(myPressedColor);
			break;
		default:
			break;
		}
	}

	void UIButton::Render()
	{
		mySprite->Render(myRect.myCenterPos.x, myRect.myCenterPos.y);

		if (myText)
			myText->Render(myRect.myCenterPos.x, myRect.myCenterPos.y);
	}

	void UIButton::Inactivate()
	{
		myState = INACTIVE;
		myPreviousState = INACTIVE;
	}

	bool UIButton::WasPressed() const
	{
		return myState == MOUSE_DOWN && myPreviousState != MOUSE_DOWN;
	}

}