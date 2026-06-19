#include "stdafx.h"
#include "IAppLayout.h"
#include "Dockable.h"

namespace Slush
{
	IAppLayout::IAppLayout(const char* aLayoutName)
		: myName(aLayoutName)
	{
	}

	IAppLayout::~IAppLayout()
	{
		myDockables.DeleteAll();
	}

	void IAppLayout::Update()
	{
		for (Dockable* dockable : myDockables)
			dockable->Update();

		OnUpdate();
	}

	void IAppLayout::Render()
	{
		OnRender();
	}

	void IAppLayout::AddDockable(Dockable* aDockable)
	{
		aDockable->myDockableID = myNextDockableID++;
		myDockables.Add(aDockable);
	}
}