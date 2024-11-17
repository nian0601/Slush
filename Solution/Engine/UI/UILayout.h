#pragma once
#include <Core\AssetParser.h>

namespace Slush
{
	struct UILayout
	{
		// https://coolors.co/db2b39-29335c-f3a712-f0cea0-534d41
		UILayout(const char* aName);

		void SaveToDisk();
		void Load(const char* aFilePath, bool aIsAbsolutePath);

		struct Button
		{
			Button()
			{
				myIdentifier = "<Button>";

				myColor = 0xff5B5548;
				FW_ARGB_To_RGBAFloat(myColor, myFloatColor);

				myHoverColor = 0xff454036;
				FW_ARGB_To_RGBAFloat(myHoverColor, myHoverFloatColor);

				myPressedColor = 0xff726A5A;
				FW_ARGB_To_RGBAFloat(myPressedColor, myPressedFloatColor);
			}

			void Parse(Slush::AssetParser::Handle aParserHandle);

			Vector2i myPosition;
			Vector2i mySize;
			int myColor;
			float myFloatColor[4];
			int myHoverColor;
			float myHoverFloatColor[4];
			int myPressedColor;
			float myPressedFloatColor[4];
			FW_String myIdentifier;
		};
		FW_GrowingArray<Button> myButtons;

		bool myIsDirty = true;
		FW_String myName;
	};
}
