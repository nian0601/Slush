#pragma once

namespace sf
{
	class RenderWindow;
}

namespace Slush
{
	class IAppLayout;
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

		// Adds aLayout to the "Layouts" menu so it becomes switchable at runtime. Window takes ownership -
		// aLayout stays alive for the Window's whole lifetime and switching the active layout never
		// deletes or recreates it, unlike SetAppLayout(). Pass aSetAsActive to make it the active layout
		// immediately (subject to the same unsaved-changes gating as a menu-driven switch).
		void AddLayout(IAppLayout* aLayout, bool aSetAsActive = false);

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

		// Switches the active layout among Window-owned myLayouts entries - never deletes anything, unlike
		// SetAppLayout().
		void SetActiveLayout(IAppLayout* aLayout);

		void RequestLayoutSwitch(IAppLayout& aTarget);

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
		IAppLayout* myPendingLayoutTarget = nullptr;

		// Layouts added via AddLayout() - owned by Window for its whole lifetime, distinct from a layout
		// set directly via SetAppLayout() (which Window deletes on the next switch/destruction instead).
		FW_GrowingArray<IAppLayout*> myLayouts;
	};
}
