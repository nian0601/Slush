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

	bool IAppLayout::HasUnsavedChanges() const
	{
		for (Dockable* dockable : myDockables)
		{
			if (dockable->HasUnsavedChanges())
				return true;
		}

		return false;
	}

	IAppLayout::CloseRequestResult IAppLayout::RequestClose()
	{
		if (myCloseWasCancelled)
		{
			myCloseWasCancelled = false;
			myCloseRequestBlocker = nullptr;
			return CloseRequestResult::Cancelled;
		}

		if (myCloseRequestBlocker && myCloseRequestBlocker->HasUnsavedChanges())
			return CloseRequestResult::StillPending;

		myCloseRequestBlocker = nullptr;

		// Under -hidewindow, each Dockable's own OnCloseRequested() detects that itself and resolves
		// synchronously (logging by asset name, not just Dockable name) instead of opening a popup that
		// would otherwise hang forever with no input to click it - see AssetEditorDockable.
		for (Dockable* dockable : myDockables)
		{
			if (dockable->HasUnsavedChanges())
			{
				myCloseRequestBlocker = dockable;
				dockable->OnCloseRequested();
				return CloseRequestResult::StillPending;
			}
		}

		return CloseRequestResult::Resolved;
	}
}