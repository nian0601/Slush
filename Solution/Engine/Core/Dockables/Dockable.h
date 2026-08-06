#pragma once

#include <FW_TypeID.h>
#include <FW_Vector2.h>

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
		// 0xFFFFFFFF marks "no type ID assigned" - FW_TypeID<Dockable>::GetID<T>() hands out real IDs
		// starting at 0, so a plain 0 default here would collide with whichever DockableBase<T> subclass
		// happens to be the first one ever queried.
		//
		// aDefaultSize: applied once via ImGuiCond_FirstUseEver, the first time this dockable's window has
		// no prior entry in the layout's imgui.ini. Has no further effect once a size gets saved (including
		// the user's own manual resize). Vector2f(0,0) (default) means "no default configured" - falls back
		// to ImGui's own default tiny window size, same as today.
		Dockable(bool aShowMenuBar = false, bool anIsClosable = false, unsigned int aDockableTypeID = 0xFFFFFFFF, Vector2f aDefaultSize = Vector2f(0.0f, 0.0f))
			: myShowMenuBar(aShowMenuBar)
			, myDockableTypeID(aDockableTypeID)
			, myIsClosable(anIsClosable)
			, myDefaultSize(aDefaultSize)
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
		Vector2f myDefaultSize = Vector2f(0.0f, 0.0f);

	private:
		unsigned int myDockableTypeID = 0xFFFFFFFF;
		bool myIsClosable = false;
		bool myIsOpen = true;
	};

	template <typename T>
	class DockableBase : public Dockable
	{
	protected:
		DockableBase(bool aShowMenuBar = false, bool anIsClosable = false, Vector2f aDefaultSize = Vector2f(0.0f, 0.0f))
			: Dockable(aShowMenuBar, anIsClosable, FW_TypeID<Dockable>::GetID<T>(), aDefaultSize)
		{}
	};
}
