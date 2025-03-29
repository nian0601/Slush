#include "stdafx.h"

#include "ComponentRegistry.h"

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
	//myComponentFactories.Fill(nullptr);
}

ComponentRegistry::~ComponentRegistry()
{
	myComponentFactories.DeleteAll();
}

