#pragma once

#include <FW_TypeID.h>
#include "Dockable.h"

namespace Slush
{
	class IAppLayout
	{
	public:
		IAppLayout(const char* aLayoutName);
		virtual ~IAppLayout();;
		void Update();
		void Render();

		const FW_String& GetName() const { return myName; }

		bool HasUnsavedChanges() const;

		enum class CloseRequestResult
		{
			StillPending,
			Cancelled,
			Resolved,
		};

		// Drives the close-confirmation flow one Dockable at a time - call every frame while a close
		// is pending. The first Dockable in list order with unsaved changes becomes "the blocker" and
		// gets OnCloseRequested() exactly once; subsequent calls just wait on it until it either
		// resolves itself (HasUnsavedChanges() goes false - re-scans for the next blocker) or a
		// Dockable calls CancelCloseRequest() from its own confirmation UI (aborts the whole attempt).
		CloseRequestResult RequestClose();

		// Called by a Dockable's own close-confirmation UI (e.g. a Cancel button) to abort the whole
		// close attempt currently in progress, not just its own popup.
		void CancelCloseRequest() { myCloseWasCancelled = true; }

		template <typename T>
		T* FindDockable()
		{
			const unsigned int typeID = FW_TypeID<Dockable>::GetID<T>();
			for (Dockable* dockable : myDockables)
			{
				if (dockable->GetDockableTypeID() == typeID)
					return static_cast<T*>(dockable);
			}

			return nullptr;
		}

		template <typename T>
		T* OpenOrCreateDockable()
		{
			if (T* existing = FindDockable<T>())
				return existing;

			T* newDockable = new T();
			AddDockable(newDockable);
			return newDockable;
		}

	protected:
		void AddDockable(Dockable* aDockable);

		virtual void OnUpdate() {};
		virtual void OnRender() {};

	private:
		FW_String myName;
		FW_GrowingArray<Dockable*> myDockables;
		int myNextDockableID = 0;

		Dockable* myCloseRequestBlocker = nullptr;
		bool myCloseWasCancelled = false;
	};
}
