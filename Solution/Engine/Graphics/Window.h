#pragma once

namespace sf
{
	class RenderWindow;
}

namespace Slush
{
	class IAppLayout;
	class IAppLayoutFactory;
	class Dockable;
	class Renderer;
	class Window
	{
	public:
		Window(unsigned int aWidth, unsigned int aHeight);
		~Window();

		bool IsOpen() const { return myShouldBeOpen; }
		void PumpEvents();
		void RenderOffscreenBufferToImGUI();

		void RequestScreenshot() { myScreenshotRequested = true; }

		void BuildEditorChrome();
		void Composite();
		void Present();

		// Single chokepoint for every way of quitting - resolves any unsaved changes (via the owning
		// IAppLayout's Dockables) before actually closing. With no unsaved changes, closes immediately.
		void Close();

		void Hide();

		void ToggleEditorUI() { myShowEditorUI = !myShowEditorUI; }
		bool IsEditorUIVisible() const { return myShowEditorUI; }

		void SetAppLayout(IAppLayout* aLayout);

		// Registers aFactory with the "Layouts" menu so its layout becomes switchable at runtime. Window
		// takes ownership - aFactory stays alive (and registered) for the Window's whole lifetime, unlike
		// the IAppLayout instances it creates on each switch.
		void RegisterLayout(IAppLayoutFactory* aFactory);

		void UpdateAppLayout();
		void RenderAppLayout();

		// Called every frame (from Engine::Run()) while a close or a menu-driven layout switch is pending,
		// until it's resolved or cancelled.
		void UpdatePendingClose();

		sf::RenderWindow* GetRenderWindow() const { return myRenderWindow; }
		Renderer& GetRenderer() const { return *myRenderer; }

		Rectf GetWindowRect() const { return myWindowRect; }
		Rectf GetGameViewRect() const { return myGameViewRect; }

	private:
		void SaveAppLayoutConfig();
		void LoadAppLayoutConfig();

		void SaveScreenshot();

		// Single point where myShouldBeOpen actually flips false, so there's always a log line marking
		// a graceful shutdown - useful for telling it apart from a crash or a forcibly-killed process.
		void ConfirmClose(const char* aReason);

		void RequestLayoutSwitch(IAppLayoutFactory& aFactory);

		// Common resolution for both a pending close and a pending layout switch, once the current
		// layout's RequestClose() reports Resolved (or there was never a layout to check). aCloseReason
		// is only used for the Close case - passed straight through to ConfirmClose().
		void ResolvePendingTransition(const char* aCloseReason);

		Vector2f GetSizeThatRespectsAspectRatio(int aWidth, int aHeight) const;

		Rectf myWindowRect;
		Rectf myGameViewRect;
		float myAspectRatio = 16.f / 9.f;
		bool myShowEditorUI = true;

		sf::RenderWindow* myRenderWindow = nullptr;
		Renderer* myRenderer = nullptr;
		bool myShouldBeOpen = true;
		bool myDisplayImGUIDemo = false;
		bool myScreenshotRequested = false;
		IAppLayout* myAppLayout = nullptr;

		// A close and a menu-driven layout switch both go through this same pending-transition flow, so
		// either one gets gated by the current layout's own unsaved-changes confirmation instead of
		// discarding silently - see UpdatePendingClose()/ResolvePendingTransition().
		enum class PendingTransition { None, Close, SwitchLayout };
		PendingTransition myPendingTransition = PendingTransition::None;
		IAppLayoutFactory* myPendingLayoutFactory = nullptr;

		FW_GrowingArray<IAppLayoutFactory*> myRegisteredLayouts;
	};
}
