#pragma once

namespace Slush
{
	struct UILayout;

	class UIWidget;
	class Input;

	class UIManager
	{
	public:
		~UIManager();

		void Update(const Input& aInput);
		void Render();

		void SetLayout(UILayout* aLayout);

		UIWidget* FindWidget(const char* aIdentifier) const;

	private:
		void ApplyLayout();

		FW_GrowingArray<UIWidget*> myWidgets;
		UILayout* myLayout = nullptr;
	};
}
