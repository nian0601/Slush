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
	};
}
