#pragma once

namespace Slush
{
	struct UILayout
	{
		struct Button
		{
			Vector2i myPosition;
			Vector2i mySize;
			int myColor;
			int myHoverColor;
			int myPressedColor;
			FW_String myIdentifier;
		};
		FW_GrowingArray<Button> myButtons;

		bool myIsDirty = true;
	};
}
