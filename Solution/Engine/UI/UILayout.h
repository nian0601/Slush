#pragma once
#include "Core/Assets/AssetParser.h"
#include "Core/Assets/DataAsset.h"
namespace Slush
{
	struct UILayout : public DataAsset
	{
		DEFINE_ASSET("UI Layout", "uilayout", "data/uilayouts/");
		using DataAsset::DataAsset;

		void OnParse(AssetParser::Handle aRootHandle) override;

		struct Button
		{
			Button();

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
