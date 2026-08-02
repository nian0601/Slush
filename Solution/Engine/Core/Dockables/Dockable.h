#pragma once

#include <FW_TypeID.h>

namespace Slush
{
	class IAppLayout;

	class Dockable
	{
		friend class Window;
		friend class IAppLayout;
	public:
		virtual ~Dockable() {}

		void Update();

		virtual const char* GetName() const = 0;

		unsigned int GetDockableTypeID() const { return myDockableTypeID; }
		bool WantsToClose() const { return myIsClosable && !myIsOpen; }

	protected:
		Dockable(bool aShowMenuBar = false, bool anIsClosable = false, unsigned int aDockableTypeID = 0)
			: myShowMenuBar(aShowMenuBar)
			, myDockableTypeID(aDockableTypeID)
			, myIsClosable(anIsClosable)
		{}

		virtual void OnUpdate() {};
		virtual void OnBuildUI() {};

		// This simply an incrementing counter for each dockable that gets created.
		// Used mainly to generate unique ImGUI-hashes so that we can have multiple copies of the same Dockable
		int myDockableID = 0;

		IAppLayout* myOwnerLayout = nullptr;

		FW_String myUniqueIDName;
		bool myShowMenuBar = false;
		bool myEnableHorizontalScrolling = false;

	private:
		unsigned int myDockableTypeID = 0;
		bool myIsClosable = false;
		bool myIsOpen = true;
	};

	template <typename T>
	class DockableBase : public Dockable
	{
	protected:
		DockableBase(bool aShowMenuBar = false, bool anIsClosable = false)
			: Dockable(aShowMenuBar, anIsClosable, FW_TypeID<Dockable>::GetID<T>())
		{}
	};
}
