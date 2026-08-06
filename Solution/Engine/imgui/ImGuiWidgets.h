#pragma once
#include "imgui.h"
#include <FW_String.h>

namespace Slush
{
	class Asset;

	// RAII scope that marks the wrapped Asset as the target of ImGuiWidgets:: calls made while it's alive.
	// Single (non-stack) current-asset pointer: BuildUI() nesting always stays within one top-level asset.
	class AssetEditScope
	{
	public:
		explicit AssetEditScope(Asset& anAsset);
		~AssetEditScope();

		AssetEditScope(const AssetEditScope&) = delete;
		AssetEditScope& operator=(const AssetEditScope&) = delete;

	private:
		Asset* myPreviousAsset;
	};

	// ImGui widget wrappers that mark the AssetEditScope-registered asset unsaved whenever they report a change.
	// Deliberately its own namespace, not Slush::ImGui - that would shadow global ::ImGui:: for every
	// unqualified ImGui::Xxx call inside namespace Slush, breaking unrelated calls everywhere.
	namespace ImGuiWidgets
	{
		bool InputText(const char* aLabel, FW_String* aString, ImGuiInputTextFlags aFlags = 0, ImGuiInputTextCallback aCallback = NULL, void* aUserData = NULL);
		bool InputText(const char* aLabel, char* aBuffer, size_t aBufferSize, ImGuiInputTextFlags aFlags = 0, ImGuiInputTextCallback aCallback = NULL, void* aUserData = NULL);
		bool InputInt(const char* aLabel, int* aValue, int aStep = 1, int aStepFast = 100, ImGuiInputTextFlags aFlags = 0);
		bool InputInt2(const char* aLabel, int aValue[2], ImGuiInputTextFlags aFlags = 0);
		bool InputFloat(const char* aLabel, float* aValue, float aStep = 0.0f, float aStepFast = 0.0f, const char* aFormat = "%.3f", ImGuiInputTextFlags aFlags = 0);
		bool InputFloat2(const char* aLabel, float aValue[2], const char* aFormat = "%.3f", ImGuiInputTextFlags aFlags = 0);
		bool Checkbox(const char* aLabel, bool* aValue);
		bool DragInt(const char* aLabel, int* aValue, float aSpeed = 1.0f, int aMin = 0, int aMax = 0, const char* aFormat = "%d", ImGuiSliderFlags aFlags = 0);
		bool DragInt2(const char* aLabel, int aValue[2], float aSpeed = 1.0f, int aMin = 0, int aMax = 0, const char* aFormat = "%d", ImGuiSliderFlags aFlags = 0);
		bool DragFloat(const char* aLabel, float* aValue, float aSpeed = 1.0f, float aMin = 0.0f, float aMax = 0.0f, const char* aFormat = "%.3f", ImGuiSliderFlags aFlags = 0);
		bool Combo(const char* aLabel, int* aCurrentItem, const char* const aItems[], int anItemsCount, int aPopupMaxHeightInItems = -1);
		bool ColorEdit4(const char* aLabel, float aColor[4], ImGuiColorEditFlags aFlags = 0);
		bool SliderInt(const char* aLabel, int* aValue, int aMin, int aMax, const char* aFormat = "%d", ImGuiSliderFlags aFlags = 0);
		bool Button(const char* aLabel, const ImVec2& aSize = ImVec2(0, 0));
		bool Selectable(const char* aLabel, bool aSelected = false, ImGuiSelectableFlags aFlags = 0, const ImVec2& aSize = ImVec2(0, 0));
	}
}
