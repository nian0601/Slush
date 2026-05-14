#pragma once

namespace Slush
{
	// https://www.youtube.com/watch?v=DYWTw19_8r4&t=100s&ab_channel=NicBarker
	// https://www.youtube.com/watch?v=by9lQvpvMIc&ab_channel=NicBarker

	struct UILayout;

	class UIWidget;
	class Input;
	class Font;

	class UIManager
	{
	public:
		UIManager(Font& aFont);
		~UIManager();

		void Update(const Input& aInput);
		void Render();

		void SetLayout(UILayout* aLayout);

		UIWidget* FindWidget(const char* aIdentifier) const;

	private:
		void ApplyLayout();
		UIWidget* FindDiscardedWidget(const char* aIdentifier) const;

		UILayout* myLayout = nullptr;
		Font& myFont;

		FW_GrowingArray<UIWidget*> myWidgets;

		// Widgets that were removed from a Layout during Hotreload is placed in this list
		// to keep the pointers valid incase someone is still holding on to a pointer.
		// The Widget wont be in 'myWidgets' anymore, so it wont be rendered or updated.
		FW_GrowingArray<UIWidget*> myDiscardedWidgets;
	};


	//////////////////////////////////////////////////////////////////////////

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

		enum InteractionFlag
		{
			NONE,
			BUTTON,
		};

		enum Alignment
		{
			TOP_LEFT,
			CENTER
		};

		UIElementStyle() { Reset(); }
		void Reset();

		void SetXSizing(UIElementStyle::SizingMode aSizingMode, int aSize = 0);
		void SetYSizing(UIElementStyle::SizingMode aSizingMode, int aSize = 0);
		void SetLayoutDirection(UIElementStyle::LayoutDirection aDirection);
		void SetAlingment(UIElementStyle::Alignment anAlignment);
		void SetPadding(int x, int y);
		void SetChildGap(int aGap);
		void SetColor(int aColor);

		void EnableButtonInteraction(int aHoverColor);

		SizingMode myXSizing;
		SizingMode myYSizing;
		LayoutDirection myLayoutDirection;
		Alignment myAlignment;

		Vector2i myMinSize;
		Vector2i myMaxSize;
		Vector2i myPadding;
		int myChildGap;
		int myColor;

		int myInteractionFlags;
		int myHoverColor;
	};

	class DynamicUIBuilder
	{
	public:
		struct RenderCommand
		{
			Vector2f myPosition;
			Vector2f mySize;
			int myColor;
			FW_String myText;
			int myTextSize = 15;
		};

		void Start();
		void Finish(FW_GrowingArray<RenderCommand>& outRenderCommands);

		void OpenElement(const char* aIdentifier = nullptr);
		void OpenElement(const UIElementStyle& aStyle);
		void OpenElement(const char* aIdentifier, const UIElementStyle& aStyle);

		void CloseElement();

		void SetStyle(const UIElementStyle& aStyle);
		UIElementStyle& GetStyle();

		// Unsure about this interface, but text-rendering in general just doesnt feel great at the moment..
		// Need to ensure that the same font is used here and when rendering.
		// But dont really want to put a whole Font-object in the Element or RenderCommmands..
		// Or do I? Its not *that* bad to just copy around a pointer?
		void SetText(const char* someText, Font& aFont, int aTextSize = 15);

		bool WasClicked(const char* aIdentifier) const;

	private:
		struct Element
		{
			Element();
			~Element();

			void Reset();

			FW_String myIdentifier;

			UIElementStyle myStyle;

			Vector2i myPosition;
			Vector2i mySize;
			Vector2i myContentSize;
			int myColor;
			FW_String myText;
			int myTextSize;
			bool myWasMouseReleased; //dont store this in the element, it needs to be in some kind of cache-thingy that persists across frames

			Element* myParent;
			FW_GrowingArray<Element*> myChildren;
		};

		Element myRoot;
		Element* myCurrentElement = nullptr;
		FW_Hashmap<FW_String, Element*> myInteractiveElements;

		void CalculateSizeAlongAxis(Element& aParent, bool aIsXAxis);
		void CalculatePositions(Element& anElement);
		bool HandleInput(Element& anElement);
		void GenerateRenderCommands(Element& anElement, FW_GrowingArray<RenderCommand>& outRenderCommands);
	};
}
