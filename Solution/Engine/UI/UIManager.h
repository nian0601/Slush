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



	class DynamicUIBuilder
	{
	public:
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

		void SetLayoutDirection(LayoutDirection aDirection);
		void SetXSizing(SizingMode aSizingMode, int aSize = 0);
		void SetYSizing(SizingMode aSizingMode, int aSize = 0);
		void SetPadding(int x, int y);
		void SetChildGap(int aGap);
		void SetColor(int aColor);

	private:
		struct Element
		{
			~Element()
			{
				myChildren.DeleteAll();
			}

			FW_String myIdentifier;

			SizingMode myXSizing = FIT;
			SizingMode myYSizing = FIT;
			LayoutDirection myLayoutDirection = LEFT_TO_RIGHT;

			Vector2i myPosition = { 0, 0 };
			Vector2i mySize = { 0, 0 };
			Vector2i myMinSize = { 0, 0 };
			Vector2i myMaxSize = { INT_MAX, INT_MAX };
			Vector2i myPadding = { 0, 0 };
			int myChildGap = 0;
			int myColor = -1;

			Element* myParent = nullptr;
			FW_GrowingArray<Element*> myChildren;
		};

		Element myRoot;
		Element* myCurrentElement = nullptr;
		Alignment myAligment = CENTER;

		void CalculateSizeAlongAxis(Element& aParent, bool aIsXAxis);
		void CalculatePositions(Element& anElement);
		void CalculateBounds(Element& aParent, Vector2i& outBounds);
		void GenerateRenderCommands(Element& anElement, FW_GrowingArray<RenderCommand>& outRenderCommands);
	};
}
