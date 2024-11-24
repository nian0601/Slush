#include "stdafx.h"

#include "UIRect.h"

#include <Core\Input.h>
#include <Graphics\RectSprite.h>

namespace Slush
{
	UIRect::UIRect()
	{
		mySprite = new RectSprite();
		mySprite->SetSize(300.f, 150.f);
		mySprite->SetFillColor(myColor);
		mySprite->SetOutlineColor(myOutlineColor);
		mySprite->SetOutlineThickness(1.f);
		myRect = MakeRectFromCenter(Vector2f(500.f, 500.f), mySprite->GetSize());
	}

	UIRect::~UIRect()
	{
		FW_SAFE_DELETE(mySprite);
	}

	void UIRect::SetSize(const Vector2i& aSize)
	{
		mySprite->SetSize(static_cast<float>(aSize.x), static_cast<float>(aSize.y));
	}

	void UIRect::SetPosition(const Vector2i& aPosition)
	{
		myRect = MakeRectFromCenter(Vector2f(static_cast<float>(aPosition.x), static_cast<float>(aPosition.y)), mySprite->GetSize());
	}

	void UIRect::SetColors(int aColor, int aOutlineColor)
	{
		myColor = aColor;
		myOutlineColor = aOutlineColor;

		mySprite->SetFillColor(myColor);
		mySprite->SetOutlineColor(myOutlineColor);
	}

	void UIRect::Render()
	{
		mySprite->Render(myRect.myCenterPos.x, myRect.myCenterPos.y);
	}
}