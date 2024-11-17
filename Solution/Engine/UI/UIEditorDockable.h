#pragma once

#include <FW_String.h>
#include <FW_FileSystem.h>

#include "Core/Dockables/Dockable.h"

#include "Core/Assets/AssetStorage.h"
#include "UILayout.h"

namespace Slush
{
	class UIEditorDockable : public Slush::Dockable
	{
	public:
		UIEditorDockable(Slush::AssetStorage<Slush::UILayout>& aUILayoutStorage);
		const char* GetName() const override { return "UI Editor"; }

	protected:
		void OnBuildUI() override;

	private:
		Slush::AssetStorage<Slush::UILayout>& myUILayoutStorage;
	};
}