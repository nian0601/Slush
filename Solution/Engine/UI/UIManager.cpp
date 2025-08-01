#include "stdafx.h"

#include "UIManager.h"
#include "UIWidget.h"
#include "UILayout.h"
#include "UIButton.h"
#include "UIRect.h"
#include <Graphics\Window.h>

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

	void DynamicUIBuilder::Start()
	{
		//Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		//Vector2f windowSize = window.GetGameViewRect().myExtents;
		Vector2f windowSize = { 1920.f, 1080.f };

		myRoot.mySize = { (int)windowSize.x, (int)windowSize.y };
		myRoot.myMinSize = { (int)windowSize.x, (int)windowSize.y };
		myRoot.myMaxSize = { (int)windowSize.x, (int)windowSize.y };
		myRoot.myXSizing = FIXED;
		myRoot.myYSizing = FIXED;
	}

	void DynamicUIBuilder::Finish(FW_GrowingArray<RenderCommand>& outRenderCommands)
	{
		CalculateSizeAlongAxis(myRoot, true);
		CalculateSizeAlongAxis(myRoot, false);

		Vector2i bounds = { 0, 0 };
		CalculateBounds(myRoot, bounds);


		if (myAligment == CENTER)
		{
			myRoot.myPosition = myRoot.mySize / 2;
			myRoot.myPosition -= bounds / 2;
		}

		CalculatePositions(myRoot);

		// Handle input

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
		int leftRightPadding = myCurrentElement->myPadding.x * 2;
		int topBottomPadding = myCurrentElement->myPadding.y * 2;

		if (myCurrentElement->myLayoutDirection == LEFT_TO_RIGHT)
		{
			myCurrentElement->mySize.x = leftRightPadding;

			for (Element* child : myCurrentElement->myChildren)
			{
				myCurrentElement->mySize.x += child->mySize.x;
				myCurrentElement->mySize.y = FW_Max(myCurrentElement->mySize.y, child->mySize.y + topBottomPadding);
			}

			int childGap = (myCurrentElement->myChildren.Count() - 1) * myCurrentElement->myChildGap;
			myCurrentElement->mySize.x += childGap;
		}
		else if (myCurrentElement->myLayoutDirection == TOP_TO_BOTTOM)
		{
			myCurrentElement->mySize.y = topBottomPadding;

			for (Element* child : myCurrentElement->myChildren)
			{
				myCurrentElement->mySize.y += child->mySize.y;
				myCurrentElement->mySize.x = FW_Max(myCurrentElement->mySize.x, child->mySize.x + leftRightPadding);
			}

			int childGap = (myCurrentElement->myChildren.Count() - 1) * myCurrentElement->myChildGap;
			myCurrentElement->mySize.y += childGap;
		}

		myCurrentElement->mySize.x = FW_Clamp(myCurrentElement->mySize.x, myCurrentElement->myMinSize.x, myCurrentElement->myMaxSize.x);
		myCurrentElement->mySize.y = FW_Clamp(myCurrentElement->mySize.y, myCurrentElement->myMinSize.y, myCurrentElement->myMaxSize.y);

		myCurrentElement = myCurrentElement->myParent;
	}

	void DynamicUIBuilder::SetLayoutDirection(LayoutDirection aDirection)
	{
		FW_ASSERT(myCurrentElement != nullptr);

		myCurrentElement->myLayoutDirection = aDirection;
	}

	void DynamicUIBuilder::SetXSizing(SizingMode aSizingMode, int aSize /*= 0*/)
	{
		FW_ASSERT(myCurrentElement != nullptr);

		myCurrentElement->myXSizing = aSizingMode;
		if (aSizingMode == FIXED)
		{
			myCurrentElement->myMinSize.x = aSize;
			myCurrentElement->myMaxSize.x = aSize;
		}
	}

	void DynamicUIBuilder::SetYSizing(SizingMode aSizingMode, int aSize /*= 0*/)
	{
		FW_ASSERT(myCurrentElement != nullptr);

		myCurrentElement->myYSizing = aSizingMode;
		if (aSizingMode == FIXED)
		{
			myCurrentElement->myMinSize.y = aSize;
			myCurrentElement->myMaxSize.y = aSize;
		}
	}

	void DynamicUIBuilder::SetPadding(int x, int y)
	{
		FW_ASSERT(myCurrentElement != nullptr);
		myCurrentElement->myPadding = { x, y };
	}

	void DynamicUIBuilder::SetChildGap(int aGap)
	{
		FW_ASSERT(myCurrentElement != nullptr);
		myCurrentElement->myChildGap = aGap;
	}

	void DynamicUIBuilder::SetColor(int aColor)
	{
		FW_ASSERT(myCurrentElement != nullptr);

		myCurrentElement->myColor = aColor;
	}

	void DynamicUIBuilder::CalculateSizeAlongAxis(Element& aParent, bool aIsXAxis)
	{
		if (aParent.myChildren.IsEmpty())
			return;

		const int parentSize = aIsXAxis ? aParent.mySize.x : aParent.mySize.y;
		const int parentPadding = aIsXAxis ? aParent.myPadding.x * 2 : aParent.myPadding.y * 2;
		const int parentChildGap = aParent.myChildGap;
		const bool sizingAlongAxis = (aIsXAxis && aParent.myLayoutDirection == LEFT_TO_RIGHT) || (!aIsXAxis && aParent.myLayoutDirection == TOP_TO_BOTTOM);

		int innerContentSize = 0;
		int totalPaddingAndChildGaps = parentPadding;
		int numGrowContainers = 0;
		FW_GrowingArray<Element*> resizables;

		for (int i = 0; i < aParent.myChildren.Count(); ++i)
		{
			Element* child = aParent.myChildren[i];
			const int childSize = aIsXAxis ? child->mySize.x : child->mySize.y;
			const SizingMode childSizingMode = aIsXAxis ? child->myXSizing : child->myYSizing;

			if (childSizingMode == GROW)
				resizables.Add(child);

			if (sizingAlongAxis)
			{
				innerContentSize += childSize;
				if (childSizingMode == GROW)
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
						const int maxSize = aIsXAxis ? child->myMaxSize.x : child->myMaxSize.y;
						int previousSize = childSize;
						if (childSize == smallest)
						{
							childSize += sizeToAdd;
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
			else // Sizing "off axis"
			{
				for (int i = 0; i < resizables.Count(); ++i)
				{
					Element* child = resizables[i];
					int& childSize = aIsXAxis ? child->mySize.x : child->mySize.y;
					const int childMinSize = aIsXAxis ? child->myMinSize.x : child->myMinSize.y;
					const int childMaxSize = aIsXAxis ? child->myMaxSize.x : child->myMaxSize.y;
					const SizingMode childSizingMode = aIsXAxis ? child->myXSizing : child->myYSizing;

					int maxSize = parentSize - parentPadding;
					if (childSizingMode == GROW)
						childSize = FW_Min(maxSize, childMaxSize);

					childSize = FW_Max(childMinSize, FW_Min(childSize, maxSize));
				}
			}
		}

		for (Element* child : aParent.myChildren)
			CalculateSizeAlongAxis(*child, aIsXAxis);
	}

	void DynamicUIBuilder::CalculatePositions(Element& anElement)
	{
		int xOffset = anElement.myPadding.x;
		int yOffset = anElement.myPadding.y;
		for (Element* child : anElement.myChildren)
		{
			child->myPosition.x = anElement.myPosition.x + xOffset;
			child->myPosition.y = anElement.myPosition.y + yOffset;
			CalculatePositions(*child);

			if (anElement.myLayoutDirection == LEFT_TO_RIGHT)
				xOffset += child->mySize.x + anElement.myChildGap;
			else if (anElement.myLayoutDirection == TOP_TO_BOTTOM)
				yOffset += child->mySize.y + anElement.myChildGap;
		}
	}

	void DynamicUIBuilder::CalculateBounds(Element& aParent, Vector2i& outBounds)
	{
		for (Element* child : aParent.myChildren)
		{
			CalculateBounds(*child, outBounds);

			outBounds.x = FW_Max(outBounds.x, child->mySize.x);
			outBounds.y = FW_Max(outBounds.y, child->mySize.y);
		}
	}

	void DynamicUIBuilder::GenerateRenderCommands(Element& anElement, FW_GrowingArray<RenderCommand>& outRenderCommands)
	{
		RenderCommand& command = outRenderCommands.Add();
		command.myPosition = anElement.myPosition;
		command.mySize = anElement.mySize;
		command.myColor = anElement.myColor;

		for (Element* child : anElement.myChildren)
			GenerateRenderCommands(*child, outRenderCommands);
	}

}