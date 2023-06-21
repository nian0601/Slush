#pragma once

#include "Dockable.h"

namespace Slush
{
	class LogDockable : public Slush::Dockable
	{
	public:
		const char* GetName() const override { return "Log"; }

	protected:
		void OnBuildUI() override;
	};
}