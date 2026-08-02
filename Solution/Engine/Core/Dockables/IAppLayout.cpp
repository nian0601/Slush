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
		for (int i = 0; i < myDockables.Count(); ++i)
		{
			Dockable* dockable = myDockables[i];
			dockable->Update();

			if (dockable->WantsToClose())
			{
				myDockables.RemoveNonCyclicAtIndex(i);
				delete dockable;
				--i;
			}
		}

		OnUpdate();
	}

	void IAppLayout::Render()
	{
		OnRender();
	}

	void IAppLayout::AddDockable(Dockable* aDockable)
	{
		aDockable->myDockableID = myNextDockableID++;
		aDockable->myOwnerLayout = this;
		myDockables.Add(aDockable);
	}
}