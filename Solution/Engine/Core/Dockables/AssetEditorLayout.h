#pragma once

#include "Core/Dockables/IAppLayout.h"

namespace Slush
{
	class AssetEditorLayout : public IAppLayout
	{
	public:
		AssetEditorLayout();
		~AssetEditorLayout();

		void OnUpdate() override;
		void OnRender() override;

	private:
	};
}
