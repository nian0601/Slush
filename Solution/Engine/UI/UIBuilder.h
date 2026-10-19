#pragma once

#include "UITheme.h"
#include <FW_Rect.h>

namespace Slush
{
	// https://www.youtube.com/watch?v=DYWTw19_8r4&t=100s&ab_channel=NicBarker
	// https://www.youtube.com/watch?v=by9lQvpvMIc&ab_channel=NicBarker

	class DebugScriptRunner;
	class Font;
	class Texture;

	struct UIInputState
	{
		Vector2i myMousePosition = { 0, 0 };
		bool myWasLeftMouseReleased = false;
		DebugScriptRunner* myDebugScriptRunner = nullptr;
	};

	namespace UITooltipUtils
	{
		Vector2i CalculatePosition(const Recti& anElementRect, const Vector2i& aTooltipSize, const Vector2i& aScreenSize);
	}

	struct UIElementStyle
	{
		enum SizingMode
		{
			FIT,
			GROW,
			FIXED
		};

		enum LayoutDirection
		{
			LEFT_TO_RIGHT,
			TOP_TO_BOTTOM,
		};

		enum Alignment
		{
			TOP_LEFT,
			CENTER
		};

		enum class AnchorPoint
		{
			TOP_LEFT, TOP_CENTER, TOP_RIGHT,
			CENTER_LEFT, CENTER, CENTER_RIGHT,
			BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT
		};

		UIElementStyle() { Reset(); }
		void Reset();

		UIElementStyle& SetXSizing(UIElementStyle::SizingMode aSizingMode, int aSize = 0);
		UIElementStyle& SetYSizing(UIElementStyle::SizingMode aSizingMode, int aSize = 0);
		UIElementStyle& SetLayoutDirection(UIElementStyle::LayoutDirection aDirection);
		UIElementStyle& SetAlingment(UIElementStyle::Alignment anAlignment);
		UIElementStyle& SetPadding(int x, int y);
		UIElementStyle& SetChildGap(int aGap);
		UIElementStyle& SetColor(int aColor);
		UIElementStyle& SetOutlineColor(int aColor);
		UIElementStyle& SetOutlineThickness(float aThickness);
		UIElementStyle& SetAnchor(AnchorPoint anAnchorPoint, const Vector2i& anOffset = { 0, 0 });

		SizingMode myXSizing;
		SizingMode myYSizing;
		LayoutDirection myLayoutDirection;
		Alignment myAlignment;
		bool myIsAnchored;
		AnchorPoint myAnchorPoint;
		Vector2i myAnchorOffset;

		Vector2i myMinSize;
		Vector2i myMaxSize;
		Vector2i myPadding;
		int myChildGap;
		int myColor;
		int myOutlineColor;
		float myOutlineThickness;
	};

	class UIBuilder
	{
	public:
		struct RenderCommand
		{
			Vector2f myPosition;
			Vector2f mySize;
			int myColor;
			int myOutlineColor;
			float myOutlineThickness;
			const Texture* myTexture = nullptr;
			Recti myTextureRect;
			FW_String myText;
			int myTextSize = 15;
		};

		void Start(const UITheme& aTheme);
		void Start(const UITheme& aTheme, const Vector2i& aScreenSize);
		void Finish();
		void Finish(const UIInputState& anInputState);
		void Finish(FW_GrowingArray<RenderCommand>& outRenderCommands);
		void GenerateRenderCommands(FW_GrowingArray<RenderCommand>& outRenderCommands);

		UIElementStyle& OpenElement(const char* aIdentifier = nullptr);
		UIElementStyle& OpenElement(const UIElementStyle& aStyle);
		UIElementStyle& OpenElement(const char* aIdentifier, const UIElementStyle& aStyle);

		void CloseElement();

		UIElementStyle& GetStyle();

		UIElementStyle& BeginPanel();
		void EndPanel();
		UIElementStyle& BeginFrame();
		void EndFrame();
		UIElementStyle& BeginButton(const char* aIdentifier, UIState aBaseState = UIState::Normal);
		void EndButton();
		void Button(const char* someText, UIState aBaseState = UIState::Normal, const Vector2i& aFixedSize = { 0, 0 });
		void Text(const char* someText, UITextRole aRole = UITextRole::Body, UITextTone aTone = UITextTone::Default);
		void TextHeader(const char* someText);
		void SetTooltip(const char* someText);
		void ScreenFade();

		void HorizontalSpacing(int aSize);
		void VerticalSpacing(int aSize);
		void Image(const Slush::Texture* aTexture, const Vector2i& aSize, const Recti& aTextureRect);

		bool WasClicked(const char* aIdentifier) const;
		bool IsMouseOverUI() const { return myIsMouseOverUI; }

	private:
		struct Element
		{
			Element();
			~Element();

			void Reset();

			FW_String myIdentifier;

			UIElementStyle myStyle;
			const UIWidgetStyle* myWidgetStyle = nullptr;
			UIState myBaseState = UIState::Normal;
			bool myIsButton = false;
			bool myIsMouseOver = false;
			bool myIsThemedText = false;
			UITextTone myTextTone = UITextTone::Default;

			Vector2i myPosition;
			Vector2i mySize;
			Vector2i myContentSize;
			int myColor;
			const Texture* myTexture;
			Recti myTextureRect;
			FW_String myText;
			int myTextSize;
			FW_String myTooltip;
			bool myWasMouseReleased; //dont store this in the element, it needs to be in some kind of cache-thingy that persists across frames

			Element* myParent;
			FW_GrowingArray<Element*> myChildren;
		};

		UIElementStyle& BeginWidget(const UIWidgetStyle& aStyle, const char* aIdentifier = nullptr);
		void AssertTheme() const;
		const UITheme* myTheme = nullptr;
		Element myRoot;
		Element myTooltip;
		bool myHasTooltip = false;
		Element* myCurrentElement = nullptr;
		bool myIsMouseOverUI = false;
		FW_Hashmap<FW_String, Element*> myInteractiveElements;
		FW_GrowingArray<Element*> myDrawOrder;

		// Unsure about this interface, but text-rendering in general just doesnt feel great at the moment..
		// Need to ensure that the same font is used here and when rendering.
		// But dont really want to put a whole Font-object in the Element or RenderCommmands..
		// Or do I? Its not *that* bad to just copy around a pointer?
		void SetText(const char* someText, Font& aFont, int aTextSize = 15);

		void CalculateSizeAlongAxis(Element& aParent, bool aIsXAxis);
		void CalculatePositions(Element& anElement);
		bool HandleInput(Element& anElement, const UIInputState& anInputState);
		bool Draws(const Element& anElement) const;
		void UpdateMouseOver(Element& anElement, const UIInputState& anInputState);
		void CollectDrawOrder(Element& anElement, FW_GrowingArray<Element*>& someAnchoredElements);
		void BuildTooltip(const Element& anElement);
		void GenerateRenderCommands(Element& anElement, FW_GrowingArray<RenderCommand>& outRenderCommands);
	};

	class RectSprite;
	class Text;

	class UIRenderer
	{
	public:
		UIRenderer(Slush::Font& aFont);
		~UIRenderer();

		void Render(const FW_GrowingArray<UIBuilder::RenderCommand>& someRenderCommands);

	private:
		Slush::RectSprite* myUISprite;
		Slush::Text* myText;
	};
}
