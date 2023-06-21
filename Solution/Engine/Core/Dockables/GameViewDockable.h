#pragma once

#include <FW_String.h>
#include <FW_FileSystem.h>

#include "Dockable.h"

namespace Slush
{
	class GameViewDockable : public Slush::Dockable
	{
	public:
		const char* GetName() const override { return "Game View"; }

	protected:
		void OnBuildUI() override;
	};
}