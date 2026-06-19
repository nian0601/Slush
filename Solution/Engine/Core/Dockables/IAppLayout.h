#pragma once

namespace Slush
{
	class Dockable;
	class IAppLayout
	{
	public:
		IAppLayout(const char* aLayoutName);
		virtual ~IAppLayout();;
		void Update();
		void Render();

		const FW_String& GetName() const { return myName; }

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
