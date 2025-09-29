#include "stdafx.h"

#include "UIManager.h"
#include "UIWidget.h"
#include "UILayout.h"
#include "UIButton.h"
#include "UIRect.h"
#include <Graphics\Window.h>
#include <Core\Input.h>

namespace Slush
{
	UIManager::UIManager(Font& aFont)
		: myFont(aFont)
	{
	}

	UIManager::~UIManager()
	{
		myWidgets.DeleteAll();
		myDiscardedWidgets.DeleteAll();
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
	
		myDiscardedWidgets.Add(myWidgets);
		myWidgets.RemoveAll();

		for (const UILayout::Rect& layoutRect : myLayout->myRects)
		{
			UIRect* rect = new UIRect();
			rect->SetSize(layoutRect.mySize);
			rect->SetPosition(layoutRect.myPosition);
			rect->SetColors(layoutRect.myColor, layoutRect.myOutlineColor);
			myWidgets.Add(rect);
		}

		for (const UILayout::Button& layoutButton : myLayout->myButtons)
		{
			UIButton* button = nullptr;
			if (UIWidget* discardedWidget = FindDiscardedWidget(layoutButton.myIdentifier.GetBuffer()))
			{
				button = static_cast<UIButton*>(discardedWidget);
				myDiscardedWidgets.RemoveCyclic(discardedWidget);
			}
			else
			{
				button = new UIButton();
			}

			button->myIdentifier = layoutButton.myIdentifier;
			button->SetSize(layoutButton.mySize);
			button->SetPosition(layoutButton.myPosition);
			button->SetColors(layoutButton.myColor, layoutButton.myHoverColor, layoutButton.myPressedColor);
			if (!layoutButton.myText.Empty() && layoutButton.myText != "<text>")
				button->SetText(layoutButton.myText.GetBuffer(), myFont);
			myWidgets.Add(button);
		}

		for (UIWidget* discarded : myDiscardedWidgets)
			discarded->Inactivate();
	}

	Slush::UIWidget* UIManager::FindDiscardedWidget(const char* aIdentifier) const
	{
		for (UIWidget* widget : myDiscardedWidgets)
		{
			if (widget->myIdentifier == aIdentifier)
				return widget;
		}

		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////

	void UIElementStyle::SetXSizing(UIElementStyle::SizingMode aSizingMode, int aSize /*= 0*/)
	{
		myXSizing = aSizingMode;
		if (aSizingMode == UIElementStyle::FIXED)
		{
			myMinSize.x = aSize;
			myMaxSize.x = aSize;
		}
	}

	void UIElementStyle::SetYSizing(UIElementStyle::SizingMode aSizingMode, int aSize /*= 0*/)
	{
		myYSizing = aSizingMode;
		if (aSizingMode == UIElementStyle::FIXED)
		{
			myMinSize.y = aSize;
			myMaxSize.y = aSize;
		}
	}

	void UIElementStyle::SetLayoutDirection(UIElementStyle::LayoutDirection aDirection)
	{
		myLayoutDirection = aDirection;
	}

	void UIElementStyle::SetAlingment(UIElementStyle::Alignment anAlignment)
	{
		myAlignment = anAlignment;
	}

	void UIElementStyle::SetPadding(int x, int y)
	{
		myPadding = { x, y };
	}

	void UIElementStyle::SetChildGap(int aGap)
	{
		myChildGap = aGap;
	}

	void UIElementStyle::SetColor(int aColor)
	{
		myColor = aColor;
	}

	void UIElementStyle::EnableButtonInteraction(int aHoverColor)
	{
		myInteractionFlags = UIElementStyle::BUTTON;
		myHoverColor = aHoverColor;
	}

	//////////////////////////////////////////////////////////////////////////

	void DynamicUIBuilder::Start()
	{
		//Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		//Vector2f windowSize = window.GetGameViewRect().myExtents;
		Vector2f windowSize = { 1920.f, 1080.f };

		myRoot.mySize = { (int)windowSize.x, (int)windowSize.y };
		myRoot.myStyle.myMinSize = { (int)windowSize.x, (int)windowSize.y };
		myRoot.myStyle.myMaxSize = { (int)windowSize.x, (int)windowSize.y };
		myRoot.myStyle.myXSizing = UIElementStyle::FIXED;
		myRoot.myStyle.myYSizing = UIElementStyle::FIXED;
		myRoot.myStyle.myColor = 0x44FFFFFF;

		myRoot.myStyle.myAlignment = UIElementStyle::CENTER;
		myRoot.myStyle.myLayoutDirection = UIElementStyle::TOP_TO_BOTTOM;
	}

	void DynamicUIBuilder::Finish(FW_GrowingArray<RenderCommand>& outRenderCommands)
	{
		FW_ASSERT(myCurrentElement == &myRoot, "Not all Elements are closed!");

		CalculateSizeAlongAxis(myRoot, true);
		CalculateSizeAlongAxis(myRoot, false);

		CalculatePositions(myRoot);

		// Handle input
		for (Element* child : myRoot.myChildren)
			HandleInput(*child);

		for (Element* child : myRoot.myChildren)
			GenerateRenderCommands(*child, outRenderCommands);
	}

	void DynamicUIBuilder::OpenElement(const char* aIdentifier /*= nullptr*/)
	{
		Element* newElement = new Element();
		if (aIdentifier)
			newElement->myIdentifier = aIdentifier;

		if (myCurrentElement)
		{
			myCurrentElement->myChildren.Add(newElement);
			newElement->myParent = myCurrentElement;
		}
		else
		{
			myRoot.myChildren.Add(newElement);
			newElement->myParent = &myRoot;
		}

		myCurrentElement = newElement;
	}

	void DynamicUIBuilder::CloseElement()
	{
		int leftRightPadding = myCurrentElement->myStyle.myPadding.x * 2;
		int topBottomPadding = myCurrentElement->myStyle.myPadding.y * 2;

		if (myCurrentElement->myStyle.myLayoutDirection == UIElementStyle::LEFT_TO_RIGHT)
		{
			myCurrentElement->mySize.x = leftRightPadding;

			for (Element* child : myCurrentElement->myChildren)
			{
				myCurrentElement->mySize.x += child->mySize.x;
				myCurrentElement->mySize.y = FW_Max(myCurrentElement->mySize.y, child->mySize.y + topBottomPadding);
			}

			int childGap = (myCurrentElement->myChildren.Count() - 1) * myCurrentElement->myStyle.myChildGap;
			myCurrentElement->mySize.x += childGap;
		}
		else if (myCurrentElement->myStyle.myLayoutDirection == UIElementStyle::TOP_TO_BOTTOM)
		{
			myCurrentElement->mySize.y = topBottomPadding;

			for (Element* child : myCurrentElement->myChildren)
			{
				myCurrentElement->mySize.y += child->mySize.y;
				myCurrentElement->mySize.x = FW_Max(myCurrentElement->mySize.x, child->mySize.x + leftRightPadding);
			}

			int childGap = (myCurrentElement->myChildren.Count() - 1) * myCurrentElement->myStyle.myChildGap;
			myCurrentElement->mySize.y += childGap;
		}

		myCurrentElement->mySize.x = FW_Clamp(myCurrentElement->mySize.x, myCurrentElement->myStyle.myMinSize.x, myCurrentElement->myStyle.myMaxSize.x);
		myCurrentElement->mySize.y = FW_Clamp(myCurrentElement->mySize.y, myCurrentElement->myStyle.myMinSize.y, myCurrentElement->myStyle.myMaxSize.y);

		myCurrentElement = myCurrentElement->myParent;
	}

	void DynamicUIBuilder::SetStyle(const UIElementStyle& aStyle)
	{
		FW_ASSERT(myCurrentElement != nullptr);
		myCurrentElement->myStyle = aStyle;
	}

	Slush::UIElementStyle& DynamicUIBuilder::GetStyle()
	{
		FW_ASSERT(myCurrentElement != nullptr);
		return myCurrentElement->myStyle;
	}

	bool DynamicUIBuilder::WasClicked(const char* aIdentifier) const
	{
		if (Element* const* element = myInteractiveElements.GetIfExists(aIdentifier))
		{
			return (*element)->myWasMouseReleased;
		}

		return false;
	}

	void DynamicUIBuilder::CalculateSizeAlongAxis(Element& aParent, bool aIsXAxis)
	{
		if (aParent.myChildren.IsEmpty())
			return;

		const int parentSize = aIsXAxis ? aParent.mySize.x : aParent.mySize.y;
		const int parentPadding = aIsXAxis ? aParent.myStyle.myPadding.x * 2 : aParent.myStyle.myPadding.y * 2;
		const int parentChildGap = aParent.myStyle.myChildGap;
		const bool sizingAlongAxis = (aIsXAxis && aParent.myStyle.myLayoutDirection == UIElementStyle::LEFT_TO_RIGHT) || (!aIsXAxis && aParent.myStyle.myLayoutDirection == UIElementStyle::TOP_TO_BOTTOM);

		int innerContentSize = 0;
		int totalPaddingAndChildGaps = parentPadding;
		int numGrowContainers = 0;
		FW_GrowingArray<Element*> resizables;

		for (int i = 0; i < aParent.myChildren.Count(); ++i)
		{
			Element* child = aParent.myChildren[i];
			const int childSize = aIsXAxis ? child->mySize.x : child->mySize.y;
			const UIElementStyle::SizingMode childSizingMode = aIsXAxis ? child->myStyle.myXSizing : child->myStyle.myYSizing;

			if (childSizingMode == UIElementStyle::GROW)
				resizables.Add(child);

			if (sizingAlongAxis)
			{
				innerContentSize += childSize;
				if (childSizingMode == UIElementStyle::GROW)
					++numGrowContainers;

				if (i > 0)
				{
					innerContentSize += parentChildGap;
					totalPaddingAndChildGaps += parentChildGap;
				}
			}
			else
			{
				innerContentSize = FW_Max(childSize, innerContentSize);
			}
		}

		if (sizingAlongAxis)
		{
			int sizeToDistribute = parentSize - parentPadding - innerContentSize;
			if (sizeToDistribute < 0)
			{
				// Handle shrinking of children...
			}
			else if (sizeToDistribute > 0 && numGrowContainers > 0)
			{
				// Make sure that 'resizables' only contain GROW-elements here.
				// For now we only add GROW-elemnts to the list, but might change in the future

				while (sizeToDistribute > 0 && resizables.Count() > 0)
				{
					int smallest = INT_MAX;
					int secondSmallest = INT_MAX;
					int sizeToAdd = sizeToDistribute;
					for (int i = 0; i < resizables.Count(); ++i)
					{
						Element* child = resizables[i];
						const int childSize = aIsXAxis ? child->mySize.x : child->mySize.y;
						if (childSize == smallest) continue;
						if (childSize < smallest)
						{
							secondSmallest = smallest;
							smallest = childSize;
						}
						if (childSize > smallest)
						{
							secondSmallest = FW_Min(secondSmallest, childSize);
							sizeToAdd = secondSmallest - smallest;
						}
					}

					sizeToAdd = FW_Min(sizeToAdd, sizeToDistribute / resizables.Count());
					sizeToAdd = FW_Max(1, sizeToAdd);

					for (int i = 0; i < resizables.Count(); ++i)
					{
						Element* child = resizables[i];
						int& childSize = aIsXAxis ? child->mySize.x : child->mySize.y;
						const int maxSize = aIsXAxis ? child->myStyle.myMaxSize.x : child->myStyle.myMaxSize.y;
						int previousSize = childSize;
						if (childSize == smallest)
						{
							childSize += sizeToAdd;

							//
							innerContentSize += sizeToAdd;
							//

							if (childSize >= maxSize)
							{
								resizables.RemoveCyclicAtIndex(i);
								i--;
							}
							sizeToDistribute -= (childSize - previousSize);
						}
					}
				}
			}
		}
		else // Sizing "off axis"
		{
			for (int i = 0; i < resizables.Count(); ++i)
			{
				Element* child = resizables[i];
				int& childSize = aIsXAxis ? child->mySize.x : child->mySize.y;
				const int childMinSize = aIsXAxis ? child->myStyle.myMinSize.x : child->myStyle.myMinSize.y;
				const int childMaxSize = aIsXAxis ? child->myStyle.myMaxSize.x : child->myStyle.myMaxSize.y;
				const UIElementStyle::SizingMode childSizingMode = aIsXAxis ? child->myStyle.myXSizing : child->myStyle.myYSizing;

				int maxSize = parentSize - parentPadding;
				if (childSizingMode == UIElementStyle::GROW)
					childSize = FW_Min(maxSize, childMaxSize);

				childSize = FW_Max(childMinSize, FW_Min(childSize, maxSize));
				innerContentSize = FW_Max(innerContentSize, childSize);
			}
		}

		if (aIsXAxis)
			aParent.myContentSize.x = innerContentSize + parentPadding;
		else
			aParent.myContentSize.y = innerContentSize + parentPadding;

		for (Element* child : aParent.myChildren)
			CalculateSizeAlongAxis(*child, aIsXAxis);
	}

	void DynamicUIBuilder::CalculatePositions(Element& anElement)
	{
		int xOffset = anElement.myStyle.myPadding.x;
		int yOffset = anElement.myStyle.myPadding.y;

		if (anElement.myStyle.myAlignment == UIElementStyle::CENTER)
		{
			xOffset += anElement.mySize.x / 2;
			yOffset += anElement.mySize.y / 2;

			xOffset -= anElement.myContentSize.x / 2;
			yOffset -= anElement.myContentSize.y / 2;
		}

		for (Element* child : anElement.myChildren)
		{
			if (child->myStyle.myAlignment == UIElementStyle::CENTER)
			{
				if (anElement.myStyle.myLayoutDirection == UIElementStyle::LEFT_TO_RIGHT)
					yOffset = (anElement.mySize.y / 2) - (child->mySize.y / 2);
				else if (anElement.myStyle.myLayoutDirection == UIElementStyle::TOP_TO_BOTTOM)
					xOffset = (anElement.mySize.x / 2) - (child->mySize.x / 2);
			}

			child->myPosition.x = anElement.myPosition.x + xOffset;
			child->myPosition.y = anElement.myPosition.y + yOffset;

			CalculatePositions(*child);

			if (anElement.myStyle.myLayoutDirection == UIElementStyle::LEFT_TO_RIGHT)
			{
				xOffset += child->mySize.x + anElement.myStyle.myChildGap;
			}
			else if (anElement.myStyle.myLayoutDirection == UIElementStyle::TOP_TO_BOTTOM)
			{
				yOffset += child->mySize.y + anElement.myStyle.myChildGap;
			}
		}
	}

	bool DynamicUIBuilder::HandleInput(Element& anElement)
	{
		const Slush::Input& input = Slush::Engine::GetInstance().GetInput();
		for (Element* child : anElement.myChildren)
		{
			if (HandleInput(*child))
				return true;
		}

		if (anElement.myStyle.myInteractionFlags != UIElementStyle::BUTTON)
			return false;

		myInteractiveElements[anElement.myIdentifier] = &anElement;

		Recti rect = MakeRectFromTopLeft(anElement.myPosition, anElement.mySize);

		if (Contains(rect, input.GetMousePosition()))
		{
			anElement.myStyle.myColor = anElement.myStyle.myHoverColor;

			if (input.WasMouseReleased(Input::LEFTMB))
			{
				anElement.myWasMouseReleased = true;
				return true;
			}
		}

		return false;
	}

	void DynamicUIBuilder::GenerateRenderCommands(Element& anElement, FW_GrowingArray<RenderCommand>& outRenderCommands)
	{
		RenderCommand& command = outRenderCommands.Add();
		command.myPosition = anElement.myPosition;
		command.mySize = anElement.mySize;
		command.myColor = anElement.myStyle.myColor;

		for (Element* child : anElement.myChildren)
			GenerateRenderCommands(*child, outRenderCommands);
	}
}