#include "stdafx.h"

#include "ComponentRegistry.h"

namespace Slush
{
	ComponentRegistry* ComponentRegistry::ourInstance = nullptr;

	ComponentRegistry& ComponentRegistry::GetInstance()
	{
		if (!ourInstance)
			ourInstance = new ComponentRegistry();

		return *ourInstance;
	}

	void ComponentRegistry::Destroy()
	{
		FW_SAFE_DELETE(ourInstance);
	}

	ComponentRegistry::ComponentRegistry()
	{
	}

	ComponentRegistry::~ComponentRegistry()
	{
		myComponentFactories.DeleteAll();
	}
}
