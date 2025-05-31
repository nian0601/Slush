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
}
