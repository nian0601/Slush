#include "stdafx.h"

#include "UIManager.h"
#include "UIWidget.h"
#include "UILayout.h"
#include "UIButton.h"

namespace Slush
{
	UIManager::~UIManager()
	{
		myWidgets.DeleteAll();
	}

	void UIManager::Update(const Input& aInput)
	{
		if (myLayout && myLayout->myIsDirty)
			ApplyLayout();

		for (UIWidget* widget : myWidgets)
			widget->Update(aInput);
	}

	void UIManager::Render()
	{
		for (UIWidget* widget : myWidgets)
			widget->Render();
	}

	void UIManager::SetLayout(UILayout* aLayout)
	{
		myLayout = aLayout;
		ApplyLayout();
	}

	Slush::UIWidget* UIManager::FindWidget(const char* aIdentifier) const
	{
		for (UIWidget* widget : myWidgets)
		{
			if (widget->myIdentifier == aIdentifier)
				return widget;
		}

		return nullptr;
	}

	void UIManager::ApplyLayout()
	{
		myLayout->myIsDirty = false;

		myWidgets.DeleteAll();

		for (const UILayout::Button& layoutButton : myLayout->myButtons)
		{
			UIButton* button = new UIButton();
			button->myIdentifier = layoutButton.myIdentifier;
			button->SetSize(layoutButton.mySize);
			button->SetPosition(layoutButton.myPosition);
			button->SetColors(layoutButton.myColor, layoutButton.myHoverColor, layoutButton.myPressedColor);
			myWidgets.Add(button);
		}
	}

}