#include "stdafx.h"
#include "ImGuiWidgets.h"
#include "imgui_user.h"
#include <Core\Assets\Asset.h>

namespace Slush
{
	static Asset* ourCurrentEditedAsset = nullptr;

	AssetEditScope::AssetEditScope(Asset& anAsset)
		: myPreviousAsset(ourCurrentEditedAsset)
	{
		ourCurrentEditedAsset = &anAsset;
	}

	AssetEditScope::~AssetEditScope()
	{
		ourCurrentEditedAsset = myPreviousAsset;
	}

	namespace ImGuiWidgets
	{
		static void MarkCurrentAssetUnsaved()
		{
			if (ourCurrentEditedAsset)
				ourCurrentEditedAsset->MarkAsUnsaved();
		}

		bool InputText(const char* aLabel, FW_String* aString, ImGuiInputTextFlags aFlags, ImGuiInputTextCallback aCallback, void* aUserData)
		{
			const bool changed = ImGui::InputText(aLabel, aString, aFlags, aCallback, aUserData);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool InputText(const char* aLabel, char* aBuffer, size_t aBufferSize, ImGuiInputTextFlags aFlags, ImGuiInputTextCallback aCallback, void* aUserData)
		{
			const bool changed = ImGui::InputText(aLabel, aBuffer, aBufferSize, aFlags, aCallback, aUserData);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool InputInt(const char* aLabel, int* aValue, int aStep, int aStepFast, ImGuiInputTextFlags aFlags)
		{
			const bool changed = ImGui::InputInt(aLabel, aValue, aStep, aStepFast, aFlags);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool InputInt2(const char* aLabel, int aValue[2], ImGuiInputTextFlags aFlags)
		{
			const bool changed = ImGui::InputInt2(aLabel, aValue, aFlags);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool InputFloat(const char* aLabel, float* aValue, float aStep, float aStepFast, const char* aFormat, ImGuiInputTextFlags aFlags)
		{
			const bool changed = ImGui::InputFloat(aLabel, aValue, aStep, aStepFast, aFormat, aFlags);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool InputFloat2(const char* aLabel, float aValue[2], const char* aFormat, ImGuiInputTextFlags aFlags)
		{
			const bool changed = ImGui::InputFloat2(aLabel, aValue, aFormat, aFlags);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool Checkbox(const char* aLabel, bool* aValue)
		{
			const bool changed = ImGui::Checkbox(aLabel, aValue);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool DragInt(const char* aLabel, int* aValue, float aSpeed, int aMin, int aMax, const char* aFormat, ImGuiSliderFlags aFlags)
		{
			const bool changed = ImGui::DragInt(aLabel, aValue, aSpeed, aMin, aMax, aFormat, aFlags);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool DragInt2(const char* aLabel, int aValue[2], float aSpeed, int aMin, int aMax, const char* aFormat, ImGuiSliderFlags aFlags)
		{
			const bool changed = ImGui::DragInt2(aLabel, aValue, aSpeed, aMin, aMax, aFormat, aFlags);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool DragFloat(const char* aLabel, float* aValue, float aSpeed, float aMin, float aMax, const char* aFormat, ImGuiSliderFlags aFlags)
		{
			const bool changed = ImGui::DragFloat(aLabel, aValue, aSpeed, aMin, aMax, aFormat, aFlags);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool Combo(const char* aLabel, int* aCurrentItem, const char* const aItems[], int anItemsCount, int aPopupMaxHeightInItems)
		{
			const bool changed = ImGui::Combo(aLabel, aCurrentItem, aItems, anItemsCount, aPopupMaxHeightInItems);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool ColorEdit4(const char* aLabel, float aColor[4], ImGuiColorEditFlags aFlags)
		{
			const bool changed = ImGui::ColorEdit4(aLabel, aColor, aFlags);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool SliderInt(const char* aLabel, int* aValue, int aMin, int aMax, const char* aFormat, ImGuiSliderFlags aFlags)
		{
			const bool changed = ImGui::SliderInt(aLabel, aValue, aMin, aMax, aFormat, aFlags);
			if (changed)
				MarkCurrentAssetUnsaved();
			return changed;
		}

		bool Button(const char* aLabel, const ImVec2& aSize)
		{
			const bool clicked = ImGui::Button(aLabel, aSize);
			if (clicked)
				MarkCurrentAssetUnsaved();
			return clicked;
		}

		bool Selectable(const char* aLabel, bool aSelected, ImGuiSelectableFlags aFlags, const ImVec2& aSize)
		{
			const bool clicked = ImGui::Selectable(aLabel, aSelected, aFlags, aSize);
			if (clicked)
				MarkCurrentAssetUnsaved();
			return clicked;
		}
	}
}
