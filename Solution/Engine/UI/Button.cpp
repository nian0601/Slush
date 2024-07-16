#include "stdafx.h"

#include "Button.h"

#include <Core\Input.h>
#include <Graphics\RectSprite.h>

namespace Slush
{
	Button::Button()
	{
		mySprite = new RectSprite();
		mySprite->SetSize(300.f, 150.f);
		mySprite->SetFillColor(1.f, 0.35f, 0.25f, 0.3f);
		myRect = MakeRectFromCenter(Vector2f(500.f, 500.f), mySprite->GetSize());
	}

	Button::~Button()
	{
		FW_SAFE_DELETE(mySprite);
	}

	void Button::Update(const Input& aInput)
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
		case Slush::Button::INACTIVE:
			mySprite->SetFillColor(1.f, 0.35f, 0.25f, 0.3f);
			break;
		case Slush::Button::HOVERED:
			mySprite->SetFillColor(1.f, 0.45f, 0.35f, 0.4f);
			break;
		case Slush::Button::MOUSE_DOWN:
			mySprite->SetFillColor(1.f, 0.55f, 0.45f, 0.5f);
			break;
		default:
			break;
		}
	}

	void Button::Render()
	{
		mySprite->Render(myRect.myCenterPos.x, myRect.myCenterPos.y);
	}

	bool Button::WasPressed() const
	{
		return myState == MOUSE_DOWN && myPreviousState != MOUSE_DOWN;
	}

}