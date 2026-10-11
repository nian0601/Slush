#include "stdafx.h"

#include "UIBuilder.h"
#include <Core\Engine.h>
#include <Graphics\Window.h>
#include <Graphics\Renderer.h>
#include <Core\Input.h>
#include "Graphics\Text.h"
#include "Graphics\RectSprite.h"

namespace Slush
{
	void UIElementStyle::Reset()
	{
		myXSizing = FIT;
		myYSizing = FIT;
		myLayoutDirection = LEFT_TO_RIGHT;
		myAlignment = TOP_LEFT;
		myIsAnchored = false;
		myAnchorPoint = AnchorPoint::TOP_LEFT;
		myAnchorOffset = { 0, 0 };

		myMinSize = { 0, 0 };
		myMaxSize = { INT_MAX, INT_MAX };
		myPadding = { 0, 0 };
		myChildGap = 0;
		myColor = 0;
		myOutlineColor = 0;
		myOutlineThickness = 0.f;
	}

	UIElementStyle& UIElementStyle::SetXSizing(UIElementStyle::SizingMode aSizingMode, int aSize /*= 0*/)
	{
		myXSizing = aSizingMode;
		if (aSizingMode == UIElementStyle::FIXED)
		{
			myMinSize.x = aSize;
			myMaxSize.x = aSize;
		}

		return *this;
	}

	UIElementStyle& UIElementStyle::SetYSizing(UIElementStyle::SizingMode aSizingMode, int aSize /*= 0*/)
	{
		myYSizing = aSizingMode;
		if (aSizingMode == UIElementStyle::FIXED)
		{
			myMinSize.y = aSize;
			myMaxSize.y = aSize;
		}

		return *this;
	}

	UIElementStyle& UIElementStyle::SetLayoutDirection(UIElementStyle::LayoutDirection aDirection)
	{
		myLayoutDirection = aDirection;
		return *this;
	}

	UIElementStyle& UIElementStyle::SetAlingment(UIElementStyle::Alignment anAlignment)
	{
		myAlignment = anAlignment;
		return *this;
	}

	UIElementStyle& UIElementStyle::SetPadding(int x, int y)
	{
		myPadding = { x, y };
		return *this;
	}

	UIElementStyle& UIElementStyle::SetChildGap(int aGap)
	{
		myChildGap = aGap;
		return *this;
	}

	UIElementStyle& UIElementStyle::SetColor(int aColor)
	{
		myColor = aColor;
		return *this;
	}

	UIElementStyle& UIElementStyle::SetOutlineColor(int aColor)
	{
		myOutlineColor = aColor;
		return *this;
	}

	UIElementStyle& UIElementStyle::SetOutlineThickness(float aThickness)
	{
		myOutlineThickness = aThickness;
		return *this;
	}

	UIElementStyle& UIElementStyle::SetAnchor(AnchorPoint anAnchorPoint, const Vector2i& anOffset)
	{
		myIsAnchored = true;
		myAnchorPoint = anAnchorPoint;
		myAnchorOffset = anOffset;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////

	void UIBuilder::Start(const UITheme& aTheme)
	{
		const Vector2f windowSize = Slush::Engine::GetInstance().GetWindow().GetRenderer().GetOffscreenBufferSize();
		Start(aTheme, { static_cast<int>(windowSize.x), static_cast<int>(windowSize.y) });
	}

	void UIBuilder::Start(const UITheme& aTheme, const Vector2i& aScreenSize)
	{
		myTheme = &aTheme;
		myIsMouseOverUI = false;

		myCurrentElement = nullptr;
		myInteractiveElements.Clear();
		myDrawOrder.RemoveAll();
		myRoot.Reset();

		myRoot.myStyle.Reset();
		myRoot.mySize = aScreenSize;
		myRoot.myStyle.myMinSize = aScreenSize;
		myRoot.myStyle.myMaxSize = aScreenSize;
		myRoot.myStyle.myXSizing = UIElementStyle::FIXED;
		myRoot.myStyle.myYSizing = UIElementStyle::FIXED;
		myRoot.myStyle.myColor = 0x44FFFFFF;

		myRoot.myStyle.myAlignment = UIElementStyle::CENTER;
		myRoot.myStyle.myLayoutDirection = UIElementStyle::TOP_TO_BOTTOM;
	}

	void UIBuilder::AssertTheme() const
	{
		FW_ASSERT(myTheme != nullptr, "Themed widgets require Start(theme)");
	}

	UIElementStyle& UIBuilder::BeginWidget(const UIWidgetStyle& aStyle, const char* aIdentifier)
	{
		AssertTheme();
		UIElementStyle& style = OpenElement(aIdentifier);
		myCurrentElement->myWidgetStyle = &aStyle;
		style.SetPadding(aStyle.myPadding.x, aStyle.myPadding.y);
		style.SetChildGap(aStyle.myChildGap);
		return style;
	}

	UIElementStyle& UIBuilder::BeginPanel()
	{
		AssertTheme();
		return BeginWidget(myTheme->myPanel);
	}

	void UIBuilder::EndPanel()
	{
		CloseElement();
	}

	UIElementStyle& UIBuilder::BeginFrame()
	{
		AssertTheme();
		return BeginWidget(myTheme->myFrame);
	}

	void UIBuilder::EndFrame()
	{
		CloseElement();
	}

	UIElementStyle& UIBuilder::BeginButton(const char* aIdentifier, UIState aBaseState)
	{
		AssertTheme();
		FW_ASSERT(aBaseState != UIState::Count, "Count is not a button state");
		UIElementStyle& style = BeginWidget(myTheme->myButton, aIdentifier);
		myCurrentElement->myBaseState = aBaseState;
		myCurrentElement->myIsButton = true;
		style.SetLayoutDirection(UIElementStyle::TOP_TO_BOTTOM);
		style.SetAlingment(UIElementStyle::CENTER);
		return style;
	}

	void UIBuilder::EndButton()
	{
		CloseElement();
	}

	void UIBuilder::Button(const char* someText, UIState aBaseState, const Vector2i& aFixedSize)
	{
		UIElementStyle& style = BeginButton(someText, aBaseState);
		if (aFixedSize.x != 0)
			style.SetXSizing(UIElementStyle::FIXED, aFixedSize.x);
		if (aFixedSize.y != 0)
			style.SetYSizing(UIElementStyle::FIXED, aFixedSize.y);
		Text(someText);
		EndButton();
	}

	void UIBuilder::Text(const char* someText, UITextRole aRole, UITextTone aTone)
	{
		AssertTheme();
		FW_ASSERT(myTheme->myFont != nullptr, "Text requires a font");
		FW_ASSERT(aRole != UITextRole::Count && aTone != UITextTone::Count);
		OpenElement().SetAlingment(UIElementStyle::CENTER);
		SetText(someText, *myTheme->myFont, myTheme->myTextSizes[static_cast<int>(aRole)]);
		myCurrentElement->myIsThemedText = true;
		myCurrentElement->myTextTone = aTone;
		CloseElement();
	}

	void UIBuilder::TextHeader(const char* someText)
	{
		AssertTheme();
		BeginWidget(myTheme->myHeader).SetXSizing(UIElementStyle::GROW).SetAlingment(UIElementStyle::CENTER);
		Text(someText, UITextRole::Header);
		CloseElement();
	}

	void UIBuilder::ScreenFade()
	{
		AssertTheme();
		OpenElement().SetColor(myTheme->myScreenFadeColor)
			.SetXSizing(UIElementStyle::GROW).SetYSizing(UIElementStyle::GROW);
		CloseElement();
	}

	void UIBuilder::Finish()
	{
		const Input& input = Engine::GetInstance().GetInput();
		UIInputState inputState;
		inputState.myMousePosition = input.GetMousePosition();
		inputState.myWasLeftMouseReleased = input.WasMouseReleased(Input::LEFTMB);
		Finish(inputState);
	}

	void UIBuilder::Finish(const UIInputState& anInputState)
	{
		FW_ASSERT(myCurrentElement == &myRoot, "Not all Elements are closed!");

		CalculateSizeAlongAxis(myRoot, true);
		CalculateSizeAlongAxis(myRoot, false);

		CalculatePositions(myRoot);
		for (Element* child : myRoot.myChildren)
			UpdateMouseOver(*child, anInputState);

		FW_GrowingArray<Element*> anchoredElements;
		CollectDrawOrder(myRoot, anchoredElements);
		for (int i = 0; i < anchoredElements.Count(); ++i)
			CollectDrawOrder(*anchoredElements[i], anchoredElements);

		// Reverse draw order gives the top-most, deepest button first.
		for (int i = myDrawOrder.Count() - 1; i >= 0; --i)
		{
			if (HandleInput(*myDrawOrder[i], anInputState))
				break;
		}
	}

	void UIBuilder::Finish(FW_GrowingArray<RenderCommand>& outRenderCommands)
	{
		Finish();
		GenerateRenderCommands(outRenderCommands);
	}

	void UIBuilder::GenerateRenderCommands(FW_GrowingArray<RenderCommand>& outRenderCommands)
	{
		for (Element* element : myDrawOrder)
			GenerateRenderCommands(*element, outRenderCommands);
	}

	void UIBuilder::CollectDrawOrder(Element& anElement, FW_GrowingArray<Element*>& someAnchoredElements)
	{
		if (&anElement != &myRoot)
			myDrawOrder.Add(&anElement);
		for (Element* child : anElement.myChildren)
		{
			if (child->myStyle.myIsAnchored)
				someAnchoredElements.Add(child);
			else
				CollectDrawOrder(*child, someAnchoredElements);
		}
	}

	UIElementStyle& UIBuilder::OpenElement(const char* aIdentifier /*= nullptr*/)
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
		return myCurrentElement->myStyle;
	}

	UIElementStyle& UIBuilder::OpenElement(const UIElementStyle& aStyle)
	{
		OpenElement();
		myCurrentElement->myStyle = aStyle;
		return myCurrentElement->myStyle;
	}

	UIElementStyle& UIBuilder::OpenElement(const char* aIdentifier, const UIElementStyle& aStyle)
	{
		OpenElement(aIdentifier);
		myCurrentElement->myStyle = aStyle;
		return myCurrentElement->myStyle;
	}

	void UIBuilder::CloseElement()
	{
		int leftRightPadding = myCurrentElement->myStyle.myPadding.x * 2;
		int topBottomPadding = myCurrentElement->myStyle.myPadding.y * 2;
		int flowChildCount = 0;
		for (Element* child : myCurrentElement->myChildren)
		{
			if (!child->myStyle.myIsAnchored)
				++flowChildCount;
		}

		if (myCurrentElement->myStyle.myLayoutDirection == UIElementStyle::LEFT_TO_RIGHT)
		{
			myCurrentElement->mySize.x = leftRightPadding;

			for (Element* child : myCurrentElement->myChildren)
			{
				if (child->myStyle.myIsAnchored)
					continue;
				myCurrentElement->mySize.x += child->mySize.x;
				myCurrentElement->mySize.y = FW_Max(myCurrentElement->mySize.y, child->mySize.y + topBottomPadding);
			}

			int childGap = FW_Max(0, flowChildCount - 1) * myCurrentElement->myStyle.myChildGap;
			myCurrentElement->mySize.x += childGap;
		}
		else if (myCurrentElement->myStyle.myLayoutDirection == UIElementStyle::TOP_TO_BOTTOM)
		{
			myCurrentElement->mySize.y = topBottomPadding;

			for (Element* child : myCurrentElement->myChildren)
			{
				if (child->myStyle.myIsAnchored)
					continue;
				myCurrentElement->mySize.y += child->mySize.y;
				myCurrentElement->mySize.x = FW_Max(myCurrentElement->mySize.x, child->mySize.x + leftRightPadding);
			}

			int childGap = FW_Max(0, flowChildCount - 1) * myCurrentElement->myStyle.myChildGap;
			myCurrentElement->mySize.y += childGap;
		}

		myCurrentElement->mySize.x = FW_Clamp(myCurrentElement->mySize.x, myCurrentElement->myStyle.myMinSize.x, myCurrentElement->myStyle.myMaxSize.x);
		myCurrentElement->mySize.y = FW_Clamp(myCurrentElement->mySize.y, myCurrentElement->myStyle.myMinSize.y, myCurrentElement->myStyle.myMaxSize.y);

		myCurrentElement = myCurrentElement->myParent;
	}

	Slush::UIElementStyle& UIBuilder::GetStyle()
	{
		FW_ASSERT(myCurrentElement != nullptr);
		return myCurrentElement->myStyle;
	}

	void UIBuilder::SetText(const char* someText, Font& aFont, int aTextSize /*= 15*/)
	{
		FW_ASSERT(myCurrentElement != nullptr);
		myCurrentElement->myText = someText;
		myCurrentElement->myTextSize = aTextSize;

		Slush::Text tempText(aFont);
		tempText.SetCharacterSize(aTextSize);

		Vector2f textSize = tempText.CalculateBounds(someText);
		myCurrentElement->myStyle.SetXSizing(UIElementStyle::FIXED, static_cast<int>(textSize.x));
		myCurrentElement->myStyle.SetYSizing(UIElementStyle::FIXED, static_cast<int>(textSize.y));
	}

	void UIBuilder::HorizontalSpacing(int aSize)
	{
		OpenElement();

		Slush::UIElementStyle& style = GetStyle();
		style.SetColor(0x00000000);
		style.SetXSizing(Slush::UIElementStyle::FIXED, aSize);
		style.SetYSizing(Slush::UIElementStyle::GROW);

		CloseElement();
	}

	void UIBuilder::VerticalSpacing(int aSize)
	{
		OpenElement();

		Slush::UIElementStyle& style = GetStyle();
		style.SetColor(0x00000000);
		style.SetXSizing(Slush::UIElementStyle::GROW);
		style.SetYSizing(Slush::UIElementStyle::FIXED, aSize);

		CloseElement();
	}

	void UIBuilder::Image(const Slush::Texture* aTexture, const Vector2i& aSize, const Recti& aTextureRect)
	{
		OpenElement();

		UIElementStyle& style = GetStyle();
		style.SetAlingment(Slush::UIElementStyle::CENTER);
		style.SetXSizing(UIElementStyle::FIXED, aSize.x);
		style.SetYSizing(UIElementStyle::FIXED, aSize.y);
		style.SetColor(0xFFFFFFFF);

		FW_ASSERT(myCurrentElement != nullptr);
		myCurrentElement->myTexture = aTexture;
		myCurrentElement->myTextureRect = aTextureRect;

		CloseElement();
	}

	bool UIBuilder::WasClicked(const char* aIdentifier) const
	{
		if (Element* const* element = myInteractiveElements.GetIfExists(aIdentifier))
		{
			return (*element)->myWasMouseReleased;
		}

		return false;
	}

	void UIBuilder::CalculateSizeAlongAxis(Element& aParent, bool aIsXAxis)
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
		int flowChildCount = 0;
		FW_GrowingArray<Element*> resizables;

		for (int i = 0; i < aParent.myChildren.Count(); ++i)
		{
			Element* child = aParent.myChildren[i];
			const int childSize = aIsXAxis ? child->mySize.x : child->mySize.y;
			const UIElementStyle::SizingMode childSizingMode = aIsXAxis ? child->myStyle.myXSizing : child->myStyle.myYSizing;
			if (child->myStyle.myIsAnchored)
			{
				if (childSizingMode == UIElementStyle::GROW)
				{
					int& size = aIsXAxis ? child->mySize.x : child->mySize.y;
					const int minSize = aIsXAxis ? child->myStyle.myMinSize.x : child->myStyle.myMinSize.y;
					const int maxSize = aIsXAxis ? child->myStyle.myMaxSize.x : child->myStyle.myMaxSize.y;
					size = FW_Clamp(parentSize, minSize, maxSize);
				}
				continue;
			}

			if (childSizingMode == UIElementStyle::GROW)
				resizables.Add(child);

			if (sizingAlongAxis)
			{
				innerContentSize += childSize;
				if (childSizingMode == UIElementStyle::GROW)
					++numGrowContainers;

				if (flowChildCount > 0)
				{
					innerContentSize += parentChildGap;
					totalPaddingAndChildGaps += parentChildGap;
				}
			}
			else
			{
				innerContentSize = FW_Max(childSize, innerContentSize);
			}
			++flowChildCount;
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

	void UIBuilder::CalculatePositions(Element& anElement)
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
			if (child->myStyle.myIsAnchored)
			{
				const int anchor = static_cast<int>(child->myStyle.myAnchorPoint);
				child->myPosition.x = anElement.myPosition.x + (anElement.mySize.x - child->mySize.x) * (anchor % 3) / 2 + child->myStyle.myAnchorOffset.x;
				child->myPosition.y = anElement.myPosition.y + (anElement.mySize.y - child->mySize.y) * (anchor / 3) / 2 + child->myStyle.myAnchorOffset.y;
				CalculatePositions(*child);
				continue;
			}
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

	bool UIBuilder::Draws(const Element& anElement) const
	{
		return anElement.myWidgetStyle || anElement.myTexture || !anElement.myText.Empty()
			|| (anElement.myStyle.myColor & 0xFF000000) != 0
			|| (anElement.myStyle.myOutlineThickness != 0.f && (anElement.myStyle.myOutlineColor & 0xFF000000) != 0);
	}

	void UIBuilder::UpdateMouseOver(Element& anElement, const UIInputState& anInputState)
	{
		anElement.myIsMouseOver = Contains(MakeRectFromTopLeft(anElement.myPosition, anElement.mySize), anInputState.myMousePosition);
		if (anElement.myIsMouseOver && Draws(anElement))
			myIsMouseOverUI = true;
		if (anElement.myIsButton)
			myInteractiveElements[anElement.myIdentifier] = &anElement;
		for (Element* child : anElement.myChildren)
			UpdateMouseOver(*child, anInputState);
	}

	bool UIBuilder::HandleInput(Element& anElement, const UIInputState& anInputState)
	{
		if (anElement.myIsButton)
		{
			if (UIThemeUtils::IsClickable(anElement.myBaseState) && anElement.myIsMouseOver && anInputState.myWasLeftMouseReleased)
			{
				anElement.myWasMouseReleased = true;
				return true;
			}
			return false;
		}

		return false;
	}

	void UIBuilder::GenerateRenderCommands(Element& anElement, FW_GrowingArray<RenderCommand>& outRenderCommands)
	{
		RenderCommand& command = outRenderCommands.Add();
		command.myPosition.x = static_cast<float>(anElement.myPosition.x);
		command.myPosition.y = static_cast<float>(anElement.myPosition.y);
		command.mySize.x = static_cast<float>(anElement.mySize.x);
		command.mySize.y = static_cast<float>(anElement.mySize.y);
		command.myColor = anElement.myStyle.myColor;
		command.myTexture = anElement.myTexture;
		command.myTextureRect = anElement.myTextureRect;
		command.myOutlineColor = anElement.myStyle.myOutlineColor;
		command.myOutlineThickness = anElement.myStyle.myOutlineThickness;
		if (anElement.myWidgetStyle)
		{
			const UIState state = UIThemeUtils::ResolveState(anElement.myBaseState, anElement.myIsMouseOver);
			const UIVisual& visual = anElement.myWidgetStyle->myVisuals[static_cast<int>(state)];
			command.myColor = visual.myColor;
			command.myOutlineColor = visual.myOutlineColor;
			command.myOutlineThickness = visual.myOutlineThickness;
		}
		if (anElement.myIsThemedText)
		{
			command.myColor = myTheme->myTextColors[static_cast<int>(anElement.myTextTone)];
			if (anElement.myTextTone == UITextTone::Default)
			{
				for (Element* ancestor = anElement.myParent; ancestor; ancestor = ancestor->myParent)
				{
					if (ancestor->myIsButton)
					{
						const UIState state = UIThemeUtils::ResolveState(ancestor->myBaseState, ancestor->myIsMouseOver);
						command.myColor = ancestor->myWidgetStyle->myTextColors[static_cast<int>(state)];
						break;
					}
				}
			}
		}
		command.myText = anElement.myText;
		command.myTextSize = anElement.myTextSize;
	}

	UIBuilder::Element::Element()
	{
		Reset();
	}

	UIBuilder::Element::~Element()
	{
		myChildren.DeleteAll();
	}

	void UIBuilder::Element::Reset()
	{
		myWidgetStyle = nullptr;
		myBaseState = UIState::Normal;
		myIsButton = false;
		myIsMouseOver = false;
		myIsThemedText = false;
		myTextTone = UITextTone::Default;
		myPosition = { 0, 0 };
		mySize = { 0, 0 };
		myContentSize = { 0, 0 };
		myColor = -1;
		myTexture = nullptr;
		myTextureRect = MakeRect(0, 0, 0, 0);
		myText = "";
		myTextSize = 15;
		myWasMouseReleased = false;

		myParent = nullptr;
		myChildren.DeleteAll();
	}

	
	//////////////////////////////////////////////////////////////////////////

	UIRenderer::UIRenderer(Slush::Font& aFont)
	{
		myUISprite = new Slush::RectSprite();
		myText = new Slush::Text(aFont);
	}

	UIRenderer::~UIRenderer()
	{
		FW_SAFE_DELETE(myText);
		FW_SAFE_DELETE(myUISprite);
	}

	void UIRenderer::Render(const FW_GrowingArray<UIBuilder::RenderCommand>& someRenderCommands)
	{
		for (const UIBuilder::RenderCommand& command : someRenderCommands)
		{
			if (command.myText.Empty())
			{
				if (command.myTexture)
				{
					myUISprite->SetTexture(*command.myTexture);
					myUISprite->SetTextureRect(command.myTextureRect);
				}
				else
				{
					myUISprite->ClearTexture();
				}

				myUISprite->SetOrigin(Slush::RectSprite::Origin::TOP_LEFT);
				myUISprite->SetPosition(command.myPosition.x, command.myPosition.y);
				myUISprite->SetSize(command.mySize.x, command.mySize.y);
				myUISprite->SetFillColor(command.myColor);
				myUISprite->SetOutlineColor(command.myOutlineColor);
				myUISprite->SetOutlineThickness(command.myOutlineThickness);
				myUISprite->Render();
			}
			else
			{
				myText->SetText(command.myText);
				myText->SetCharacterSize(command.myTextSize);
				myText->SetColor(command.myColor);
				myText->SetPosition(command.myPosition.x, command.myPosition.y);
				myText->Render();
			}
		}
	}

}
