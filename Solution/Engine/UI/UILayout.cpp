#include "stdafx.h"
#include "UILayout.h"
#include <Core/Assets/AssetParser.h>

namespace Slush
{
	void UILayout::OnParse(AssetParser::Handle aRootHandle)
	{
		Slush::AssetParser::Handle buttonsHandle = aRootHandle.ParseChildElement("buttons");
		Slush::AssetParser::Handle rectsHandle = aRootHandle.ParseChildElement("rects");

		if (aRootHandle.IsReading())
		{
			myButtons.RemoveAll();

			int childCount = buttonsHandle.GetNumChildElements();
			for (int i = 0; i < childCount; ++i)
			{
				Button& button = myButtons.Add();
				button.Parse(buttonsHandle.GetChildElementAtIndex(i));
			}

			myRects.RemoveAll();
			childCount = rectsHandle.GetNumChildElements();
			for (int i = 0; i < childCount; ++i)
			{
				Rect& rect = myRects.Add();
				rect.Parse(rectsHandle.GetChildElementAtIndex(i));
			}
			
		}
		else
		{
			for (Button& button : myButtons)
				button.Parse(buttonsHandle.ParseChildElement("button"));

			for (Rect& rect: myRects)
				rect.Parse(rectsHandle.ParseChildElement("rect"));
		}
	}

	//////////////////////////////////////////////////////////////////////////

	UILayout::Button::Button()
	{
		myIdentifier = "<Button>";
		myText = "<text>";


		// https://coolors.co/db2b39-29335c-f3a712-f0cea0-534d41
		myColor = 0xff5B5548;
		FW_ARGB_To_RGBAFloat(myColor, myFloatColor);

		myHoverColor = 0xff454036;
		FW_ARGB_To_RGBAFloat(myHoverColor, myHoverFloatColor);

		myPressedColor = 0xff726A5A;
		FW_ARGB_To_RGBAFloat(myPressedColor, myPressedFloatColor);
	}

	void UILayout::Button::Parse(Slush::AssetParser::Handle aHandle)
	{
		aHandle.ParseVec2iField("position", myPosition);
		aHandle.ParseVec2iField("size", mySize);
		aHandle.ParseStringField("text", myText);
		aHandle.ParseIntField("color", myColor);
		aHandle.ParseIntField("hoverColor", myHoverColor);
		aHandle.ParseIntField("pressedColor", myPressedColor);
		aHandle.ParseStringField("identifier", myIdentifier);

		if (aHandle.IsReading())
		{
			FW_ARGB_To_RGBAFloat(myColor, myFloatColor);
			FW_ARGB_To_RGBAFloat(myHoverColor, myHoverFloatColor);
			FW_ARGB_To_RGBAFloat(myPressedColor, myPressedFloatColor);
		}
	}

	UILayout::Rect::Rect()
	{
		// https://coolors.co/db2b39-29335c-f3a712-f0cea0-534d41
		myColor = 0xff5B5548;
		FW_ARGB_To_RGBAFloat(myColor, myFloatColor);

		myOutlineColor = 0xff454036;
		FW_ARGB_To_RGBAFloat(myOutlineColor, myFloatOutlineColor);
	}

	void UILayout::Rect::Parse(Slush::AssetParser::Handle aHandle)
	{
		aHandle.ParseVec2iField("position", myPosition);
		aHandle.ParseVec2iField("size", mySize);
		aHandle.ParseIntField("color", myColor);
		aHandle.ParseIntField("outlineColor", myOutlineColor);

		if (aHandle.IsReading())
		{
			FW_ARGB_To_RGBAFloat(myColor, myFloatColor);
			FW_ARGB_To_RGBAFloat(myOutlineColor, myFloatOutlineColor);
		}
	}

}
