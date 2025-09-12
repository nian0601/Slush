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

		void SetLayoutDirection(UIElementStyle::LayoutDirection aDirection);
		void SetXSizing(UIElementStyle::SizingMode aSizingMode, int aSize = 0);
		void SetYSizing(UIElementStyle::SizingMode aSizingMode, int aSize = 0);
		void SetPadding(int x, int y);
		void SetChildGap(int aGap);
		void SetColor(int aColor);

		void EnableButtonInteraction(int aHoverColor);

		SizingMode myXSizing = FIT;
		SizingMode myYSizing = FIT;
		LayoutDirection myLayoutDirection = LEFT_TO_RIGHT;

		Vector2i myMinSize = { 0, 0 };
		Vector2i myMaxSize = { INT_MAX, INT_MAX };
		Vector2i myPadding = { 0, 0 };
		int myChildGap = 0;
		int myColor = -1;

		int myInteractionFlags = NONE;
		int myHoverColor = -1;
	};

	class DynamicUIBuilder
	{
	public:
		enum Alignment
		{
			TOP_LEFT,
			CENTER
		};

		struct RenderCommand
		{
			Vector2i myPosition;
			Vector2i mySize;
			int myColor;
		};

		void Start();
		void Finish(FW_GrowingArray<RenderCommand>& outRenderCommands);

		void OpenElement(const char* aIdentifier = nullptr);
		void CloseElement();

		void SetStyle(const UIElementStyle& aStyle);
		UIElementStyle& GetStyle();

		bool WasClicked(const char* aIdentifier) const;

	private:
		struct Element
		{
			~Element()
			{
				myChildren.DeleteAll();
			}

			FW_String myIdentifier;

			UIElementStyle myStyle;

			Vector2i myPosition = { 0, 0 };
			Vector2i mySize = { 0, 0 };
			int myColor = -1;

			bool myWasMouseReleased = false; //dont store this in the element, it needs to be in some kind of cache-thingy that persists across frames

			Element* myParent = nullptr;
			FW_GrowingArray<Element*> myChildren;
		};

		Element myRoot;
		Element* myCurrentElement = nullptr;
		FW_Hashmap<FW_String, Element*> myInteractiveElements;
		Alignment myAligment = CENTER;

		void CalculateSizeAlongAxis(Element& aParent, bool aIsXAxis);
		void CalculatePositions(Element& anElement);
		void CalculateBounds(Element& aParent, Vector2i& outBounds);
		bool HandleInput(Element& anElement);
		void GenerateRenderCommands(Element& anElement, FW_GrowingArray<RenderCommand>& outRenderCommands);
	};
}
