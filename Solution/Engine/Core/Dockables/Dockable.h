#pragma once

namespace Slush
{
	class Dockable
	{
		friend class Window;
	public:
		virtual ~Dockable() {}

		void Update();

		virtual const char* GetName() const = 0;

	protected:
		Dockable(bool aShowMenuBar = false)
			: myShowMenuBar(aShowMenuBar)
		{}

		virtual void OnUpdate() {};
		virtual void OnBuildUI() {};

		// This simply an incrementing counter for each dockable that gets created.
		// Used mainly to generate unique ImGUI-hashes so that we can have multiple copies of the same Dockable
		int myDockableID = 0;

		FW_String myUniqueIDName;
		bool myShowMenuBar = false;
		bool myEnableHorizontalScrolling = false;
	};
}
