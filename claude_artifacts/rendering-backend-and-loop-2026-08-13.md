# Rendering backend & engine-loop discussion — 2026-08-13

Working notes from a design conversation covering: what it would take to move Slush off
SFML's renderer, and a chain of structural findings that fell out of investigating it —
the main loop, the `Window` class, and the `IApp` / `IAppLayout` relationship.

**Filed during the discussion:** #36 (new, P0), #12 (rescoped and retitled).

Nothing here was verified by running the engine — the session was on Linux and the build
is Windows-only. Everything is from reading the code. One claim in particular (#36) needs
a runtime check; it's flagged where it appears.

---

## 1. Where Slush's rendering stands today

Slush is a direct SFML consumer, not a custom renderer with SFML as one backend:

- `Renderer` wraps `sf::RenderTarget`/`RenderTexture`/`CircleShape`/`RectangleShape`/`VertexArray`.
  Its five primitives (`RenderLine` ×2, `RenderTriangle`, `RenderRect`, `RenderCircle`)
  each build an SFML shape and draw immediately.
- `Window` owns `sf::RenderWindow*` directly; `BaseSprite`/`RectSprite`/`CircleSprite`,
  `Texture`, `Text`, `Font` all hold SFML types as members in *public* headers.
- No `Shader`, `Material`, `Mesh`, `VertexBuffer`, `RHI`, or `IRenderer` exists anywhere.
  `sf::Shader` isn't referenced at all.
- Purely 2D. No camera/projection/view-matrix code, no `Matrix4`. `FW_Vector3` exists but
  isn't used for rendering.
- Only two graphics asset types: `Texture` (wraps `sf::Texture`) and `Animation`
  (spritesheet-based). No shader or material assets.
- Editor path: the game renders to a fixed 1920×1080 offscreen `sf::RenderTexture`, which
  `GameViewDockable` blits into an ImGui panel.

**Consequence:** there is no seam to swap. A backend change today means replacing SFML's
renderer outright and touching every public header that names an `sf::` type.

---

## 2. Modern APIs vs. D3D9 / D3D11 / OpenGL

What's actually new, roughly in order of what bites first:

**Explicit resource state / barriers.** D3D11 and GL infer transitions; D3D12 and Vulkan
make you declare them (`ResourceBarrier` / `vkCmdPipelineBarrier`, with access masks and
pipeline stages in Vulkan). This is the biggest new bug class — mistakes give corruption
or validation-layer errors, not clean crashes.

**Manual GPU memory.** No more "just create a resource." D3D12 has committed/placed
resources over `ID3D12Heap`; Vulkan has raw `VkDeviceMemory` you sub-allocate. In practice
everyone uses D3D12MA or VMA rather than hand-rolling an allocator.

**Pipeline State Objects.** D3D11's piecemeal state setters collapse into one immutable
object baked ahead of time. Cheaper at draw time, but you now manage a combinatorial set
of PSOs (or a hash/cache keyed on state).

**Command lists + queues + fences.** You record commands and submit to a queue; graphics,
compute and copy are independent queues. This is what unlocks multithreaded rendering —
but only if you build a job system to use it. Naively it's more code for the same
single-threaded result.

**Descriptor binding.** Slot-based `SetShaderResources` is gone. D3D12 uses descriptor
heaps + root signatures; Vulkan uses descriptor sets + pipeline layouts. Similar to each
other, both stricter and more declarative than D3D11.

**Shader toolchain.** D3D12 wants HLSL → DXIL via DXC (not FXC). Vulkan wants SPIR-V,
which DXC can also emit from HLSL via `-spirv`.

**Recommendation: D3D12.** Windows-only means Vulkan's portability buys nothing, and
D3D12 has friendlier defaults (no instance/extension negotiation, no manual render-pass
objects) plus PIX, which is an excellent debugger. If the goal is learning the explicit
model, D3D12 gets there with less incidental verbosity.

---

## 3. Your plan

> Standalone renderer first (2D + working ImGui) → abstraction layer over SFML with SFML
> as first consumer → port the renderer in, kept as a separate project → maybe 3D later.

The sequencing holds up. Isolating the D3D12 learning curve from Slush's SFML coupling is
right, and deferring 3D is right — the explicit-API model (barriers, PSOs, command lists,
descriptor heaps) shows up fully in 2D. Depth, culling and projection add nothing to what
you're trying to learn.

Two adjustments, both of which shrink the work:

1. **The abstraction step is smaller than it looks**, because #11 already does most of it
   (§5).
2. **You may not need to replace SFML at all** — only its renderer (§4).

---

## 4. SFML 3.0.2 changes the port's scope

The vendored SFML is **3.0.2**, which ships `sf::WindowBase` — a window with *no OpenGL
context* — and `getNativeHandle()` returning the HWND on Windows.

So the port doesn't have to mean "replace SFML." Keep SFML for windowing, input and audio,
swap `sf::RenderWindow` → `sf::WindowBase`, and hang a D3D12 swapchain off the HWND. The
abstraction layer then only has to cover drawing and GPU resources — not window lifecycle,
event pumping, or audio.

(SFML 3 also ships `Vulkan.hpp` with surface-creation helpers, if that route ever appeals.)

---

## 5. Issue #11 already does the important groundwork

#11 converts `Renderer` from immediate drawing to per-target queues flushed once per frame,
and routes `BaseSprite` and `Text` through it. It's `status:planned` with a 3-phase
breakdown, and explicitly names "a future SFML-agnostic/custom rendering backend" as
motivation.

Two of its design decisions matter a lot for the backend goal:

- **`RenderCommand` stays private to `Renderer`** and all public signatures are unchanged,
  so the command vocabulary can be revised later with zero call-site churn.
- **Per-target queues keyed on the active target**, flushed once per frame, is structurally
  the record → submit shape you want.

After #11 lands, the ~74 direct-SFML draw call sites in game/engine code (`SpriteComponent`,
`Tilemap`, UI widgets, `Card`, physics/navmesh debug-draw) are gone.

**One thing to watch:** the new textured overloads take `outlineColor` and
`outlineThickness` as *public* parameters. Those are `sf::Shape` concepts with no GPU
primitive equivalent — in D3D12 an outline is two quads or a shader trick. Everything else
in that signature (rect, texture, texture-rect, rotation, fill color) maps cleanly. It's
the one piece of SFML's feature vocabulary the issue bakes into the public API rather than
keeping private.

---

## 6. The Update/Render split — what it's actually for

#12 originally assumed the split existed "to enforce immediate-mode draw ordering." That's
only partly true, and it matters because the rest won't dissolve when #11 lands.

**At `IApp` — vestigial.** `Engine::Run()` calls `anApp.Update()` and `anApp.Render()` on
adjacent lines with nothing between them. ActionGame's `App::Render()` is empty. Removing
it is a two-line deletion of dead virtuals.

**At `StateStack` — load-bearing.** `StateStack::Update()` updates only the top state,
while `StateStack::Render()` recurses *down* through `AllowPassThroughRender()` and renders
bottom-to-top. All five ActionGame overlay states (`PauseState`, `UpgradeWeaponState`,
`UpgradeStatsState`, `GameOverState`, `CharacterSelectionState`) return `true`. That's
1 update : N renders — how the paused game stays visible behind a menu. It can't be
expressed with a single merged entry point per state, and it has nothing to do with
immediate mode.

**No layers needed in the render queue.** Because the pass-through recursion submits
bottom-to-top in a single pass, submission order already equals correct back-to-front draw
order. #11's decision 9 (submission order, no z-index) is fine as-is. Refilling the queue
every frame is also the right model for threading: full rebuild → swap → hand off beats a
retained/layered scheme.

---

## 7. The main loop

### How the frame actually executes

**`Window::PumpEvents()`** — *the loop condition*: shutdown check; SFML event poll →
`ImGui::SFML::ProcessEvent`, `Closed`/`Resized`; reads input (`WasKeyPressed(HYPHEN)` →
`ToggleEditorUI()`); `ImGui::SFML::Update()` **begins the ImGui frame**.

**`Engine::Run()` body**: `Time::Update()`, `Logger::Update()`; input refresh
(`UpdateKeyboard` / `PollDebugInputFile` / `UpdateMouse` / `RemapMousePosition`);
`anApp.Update()`, `anApp.Render()`; `RenderAppLayout()` → `GameLayout::OnRender()` →
`StartOffscreenBuffer` / **`StateStack::Render()`** / `EndOffscreenBuffer`; F10 screenshot
request; `Renderer::RenderFade()` (ticks the fade timer *and* draws).

**`Window::Present()`**: `UpdatePendingClose()`; **editor branch only** → menubar,
dockspace, demo, `IAppLayout::Update()` → dockables + `GameLayout::OnUpdate()` →
**`StateStack::Update()`**, then `ImGui::SFML::Render()`; non-editor branch → composite
the offscreen buffer via a local `sf::RectangleShape`; `display()`; screenshot; `clear()`.

### Problems

| # | Problem |
|---|---|
| 1 | Simulation gated on editor-UI visibility — filed as **#36** |
| 2 | Update runs *after* Render in the same iteration; presented frame reflects the previous iteration's simulation |
| 3 | The loop condition does a quarter of the frame; the ImGui frame spans condition → body → `Present()` |
| 4 | Input read before it's refreshed — the HYPHEN check precedes `UpdateKeyboard()`, so the toggle acts on last frame's state |
| 5 | `myGameViewRect` written in `Present()`, consumed by `RemapMousePosition()` next iteration — one-frame-stale mouse mapping |
| 6 | `RenderFade()` both ticks the timer and draws, and draws straight to `myOffscreenBuffer`, bypassing the active-target concept |
| 7 | Two compositing paths with different mechanisms; `UpdatePendingClose()` mutates `myShowEditorUI` as a side effect |
| 8 | `Dockable::Update()` mixes `OnUpdate()` logic with ImGui widget construction |

### Proposed shape

Split logic from ImGui-widget-building at each layer:

- `IAppLayout::Update()` → `Update()` (`OnUpdate()` only, unconditional) + `BuildUI()`
  (dockable loop, editor-only)
- `Dockable::Update()` → `Update()` (→ `OnUpdate()`) + `BuildUI()`
- `Window::Present()` → `BuildEditorChrome()` + `Composite()` + `Present()`
- `Window::PumpEvents()` → `IsOpen()` (pure predicate) + `PumpEvents()` (events only)
- `Renderer::RenderFade()` → `UpdateFade()` (tick) + `RenderFade()` (draw)

```cpp
while (myWindow->IsOpen())
{
    myWindow->PumpEvents();     // events only
    BeginFrame();               // Time, Logger, ImGui::SFML::Update
    UpdateInput();              // keyboard/mouse/debug-file, remap, editor toggle
    UpdateSimulation(anApp);    // anApp.Update() + layout->Update(), fade tick
    RenderFrame();              // StartOffscreen / layout->Render() / fade draw / EndOffscreen / ProcessRenderQueue()
    BuildEditorUI();            // menubar, dockspace, layout->BuildUI()
    CompositeAndPresent();      // offscreen -> window, ImGui::SFML::Render, display, screenshot, clear
}
```

`BuildEditorUI()` sits after `RenderFrame()` to preserve today's ordering, where
`GameViewDockable` reads an already-filled offscreen buffer.

**Moving simulation ahead of render is a behavior change** — it removes the effective
one-frame lag. Needs before/after screenshot verification, not treated as pure refactor.

`UpdateSimulation()` and `RenderFrame()` become the two halves #12's threading work would
run concurrently, with `ProcessRenderQueue()` / buffer swap as the handoff. Today there's
no point in the loop where that boundary could even be drawn.

---

## 8. The `Window` class

301 lines doing six jobs:

1. **OS window + event pump** — the only one that's genuinely a window
2. **ImGui bootstrap + editor chrome** — `ImGui::SFML::Init` in the constructor, docking
   flags, menubar, dockspace, demo toggle, `myShowEditorUI`
3. **App layout ownership** — holds `IAppLayout`, saves/loads `ImGUILayouts/<name>.ini`
4. **Close negotiation** — unsaved-changes flow, pending-close state machine
5. **Screenshot** — request flag, backbuffer capture, rotation to `screenshot_previous.png`
6. **Presentation & letterboxing** — offscreen→window compositing (two paths),
   `display()`/`clear()`, `myGameViewRect`, aspect-ratio math

### The seam that matters

#9 (splitting `Renderer` out of `Window`) landed, but the line was drawn at "drawing
primitives" rather than "who owns the device" — both classes hold `sf::RenderWindow*`, and
`Present()` still calls `draw()`, `display()` and `clear()` itself.

The useful seam is **OS surface vs. swapchain**:

- **Survives a backend swap:** window creation, event polling, resize, close, size/rect.
  SFML keeps doing all of it via `sf::WindowBase`.
- **Gets rewritten entirely:** offscreen targets, compositing, present/clear, screenshot
  readback, arguably the letterbox math.

The second group lives almost entirely in `Window` today, so a port would tear through a
class that otherwise had no reason to change. Moving it into `Renderer` now leaves `Window`
backend-agnostic and confines the port to one class.

Three specifics:

- **`GetRenderWindow()` is the accessor to kill.** It hands out the raw
  `sf::RenderWindow*`; consumers are `Input::UpdateMouse(sf::RenderWindow&)`, ImGui-SFML,
  and `Renderer`'s constructor. Target end state: `Window` exposes a native handle and a
  size, nothing SFML-typed in its public header. Input should receive engine-native mouse
  coordinates rather than being handed an SFML object to query.
- **Screenshot is a GPU readback** — trivial in SFML, but in D3D12 it's a readback heap, a
  fence wait and row-pitch handling. Since the verification workflow leans on F10
  screenshots, getting it behind a `Renderer` interface early gives the port an obvious
  place to reimplement it.
- **`myGameViewRect` is viewport state with three owners** — written in two places inside
  `Present()`, read by input remapping and by the game-view dockable. Wants to be computed
  once per frame and owned by `Renderer`.

Also note #10 (multiple offscreen render targets) will make the two-path compositing branch
worse if it stays where it is.

---

## 9. `EditorShell`

Jobs 2, 3 and 4 above are editor-shell concerns that live on `Window` by accident. Pulling
them out removes roughly half the file.

**Owns:** the ImGui runtime lifecycle (init / new-frame / render / shutdown, docking
config), event forwarding to ImGui, the chrome (menubar, dockspace, demo toggle),
visibility (`myShowEditorUI` / HYPHEN), per-layout `.ini` persistence, and the
unsaved-changes/close negotiation across dockables.

**Does *not* own the layout.** The layout is the *app root* — it must always be updated and
rendered — so `Engine` owns it and drives it unconditionally. `EditorShell` only calls
`layout->BuildUI()` when visible, and the dockable *list* stays with the layout.

```
UpdateSimulation()  -> anApp.Update() + layout->Update()          // always
RenderFrame()       -> layout->Render() + queue flush             // always
BuildEditorUI()     -> editorShell.BuildUI() -> layout->BuildUI() // if visible
```

Gating then exists in exactly one place, on the one phase that's genuinely editor-only —
which makes the #36 class of bug structurally impossible rather than fixed-once.

**Why this helps the rendering goal:** ImGui's *backend binding* is currently smeared
across three places in `Window` (init in the constructor, event-processing and new-frame in
`PumpEvents()`, render in `Present()`). Swapping imgui-SFML for `imgui_impl_win32` +
`imgui_impl_dx12` becomes a one-file change. It's also exactly the seam where the standalone
renderer project's ImGui integration would plug in later.

**One coupling to define deliberately:** `GameViewDockable` needs the offscreen target as an
`ImTextureID`. Today that's
`Window::RenderOffscreenBufferToImGUI()` → `Renderer::GetOffscreenBuffer()->getTexture().getNativeHandle()`.
After a port it's a D3D12 descriptor handle. Make it one named function on `Renderer` and
the editor↔backend contract stays one function wide.

---

## 10. `IApp` vs. `IAppLayout`

### Why both exist: two lifetimes

- **Program lifetime** — register asset types, load assets, fonts, globals, and decide
  which layout is active
- **Layout lifetime** — a named composition of dockables plus that mode's per-frame logic

`IApp::Update()` isn't dead weight: keys `1`/`2` do live layout switching. That's the one
thing a layout can't safely do for itself.

**Hazard to design around:** `Window::SetAppLayout()` does `FW_SAFE_DELETE(myAppLayout)`
immediately. It's safe today only because `anApp.Update()` runs before anything touches the
layout that frame. If a layout — or a dockable inside it — called it during
`myAppLayout->Update()`, it would delete itself mid-update. **Wiring up the dead
`ImGui::Selectable("Game")` menu items naively is a use-after-free.** Switching needs to
become "store a pending mode, apply at a safe point in the frame."

### Option A — merge

```cpp
class IAppMode
{
public:
    IAppMode(const char* aName);
    virtual ~IAppMode();

    const FW_String& GetName() const;

    void Update();    // -> OnUpdate(), always
    void Render();    // -> OnRender(), always
    void BuildUI();   // dockable loop, editor-only

    // AddDockable / FindDockable / OpenOrCreateDockable
    // HasUnsavedChanges / RequestClose / CancelCloseRequest — unchanged

protected:
    virtual void OnEnter() {}
    virtual void OnExit()  {}
    virtual void OnUpdate() {}
    virtual void OnRender() {}
};
```

Program lifetime moves to `main()`:

```cpp
int main(int argc, char** argv)
{
    FW_UnitTestSuite::RunTests();
    CommandLineArgs::GetInstance().Parse(argc, argv);

    Engine& engine = Engine::GetInstance();
    engine.Initialize();

    ActionGameAssets::Register();     // was IApp::Initialize()
    ActionGameGlobals::Create();

    engine.SetMode(new GameMode());   // deferred-applied
    engine.Run();                     // no IApp parameter

    ActionGameGlobals::Destroy();     // was IApp::Shutdown()
}
```

Switching becomes `Engine::SetMode()`, driven from the Workspaces menu — where that UI
wanted to live anyway.

**Cost:** `main()` takes on ordering responsibility `Engine::Run` used to enforce, and
there's no per-frame hook above the mode.

### Option B — keep both, narrowed

- `IApp::Render()` — **delete**. Nothing renders outside a mode. (BossMonster and
  TopDownGame use it and would need porting — small.)
- `IApp::Update()` — keep, narrowed to explicitly mode-independent concerns: switching,
  global hotkeys. Runs before the mode's update.
- `IApp::Initialize()`/`Shutdown()` — unchanged; this is where `IApp` earns its keep,
  getting ordering right relative to `Engine`'s own init/shutdown.
- `IAppLayout` — rename, move ownership from `Window` to `Engine`, add deferred
  `Engine::SetMode()`, split `Update()`/`BuildUI()`.

### On the name

"Layout" already means two things: the ImGui docking arrangement persisted to
`ImGUILayouts/<name>.ini`, and the app mode. **`Workspace`** resolves it — a set of panels
plus what you're doing in them — and leaves "layout" free to mean the docking arrangement,
so `Workspaces/Game.ini` reads unambiguously. `AppMode` works too, just less evocative of
the compose-dockables idea.

### Recommendation

**B is a strict prefix of A.** Shared work: rename to `Workspace`, move ownership from
`Window` to `Engine`, add deferred `SetMode`, split `Update()`/`BuildUI()`, delete
`IApp::Render()`.

Do that first, then look at what's left in `IApp::Update()`. If switching has moved to the
Workspaces menu and global hotkeys went to a debug dockable, it's empty — and `IApp` is
then just `Initialize`/`Shutdown`, which is what `main()` already is. The merge falls out
for free, decided with evidence rather than upfront.

The one thing that would keep you at B deliberately: wanting a guaranteed per-frame hook
that runs regardless of which workspace is active.

---

## Issues filed / updated

**#36 — Game simulation and dockable updates stop while the editor UI is hidden**
(`priority:p0`, `project:engine`, `status:unplanned`) — new.

`myAppLayout->Update()` has exactly one call site, inside `if (myShowEditorUI)` in
`Window::Present()` (there since 2026-07-28), while `RenderAppLayout()` runs
unconditionally. So hiding the editor should stop every `Dockable::Update()` and
`GameLayout::OnUpdate()` → `StateStack::Update()` while rendering continues.

Confirmed during discussion as **unintentional** — the intended rule is that the active
layout is always updated and rendered, with editor-specific logic living in dockables. The
fix already written into the issue (split `IAppLayout::Update()` into `Update()` +
`BuildUI()`) expresses exactly that rule.

Only ActionGame can currently hit it, since it's the only game using the layout approach —
BossMonster is stale and TopDownGame is a minimal navmesh spike, both still driving from
`App::Update()`/`Render()`. That's incidental, not architectural.

**⚠ Still needs a runtime check:** launch ActionGame with `SkipStartScreen`, press HYPHEN,
see whether enemies keep moving. If they do, the reading is wrong and the issue should be
closed.

**#12 — Restructure the main loop; add threaded rendering; re-examine the Update/Render
interface exposed to games** — retitled and rescoped.

Kept the original threading scope, added the §6 findings on where the split lives and the
§7 loop restructure (frame trace, eight problems, proposed phase split, target
`Engine::Run()`). Still `blocked` on #11.

---

## Open decisions

1. **Confirm the #36 freeze empirically** — everything else about it is settled.
2. **Add a note to #36** recording that the gating was unintentional and stating the
   intended rule, so it isn't re-derived from the code later. *(Offered, not yet done.)*
3. **Do you want a per-frame hook above the workspace?** The only real differentiator
   between merging `IApp`/`IAppLayout` and keeping both.
4. **Do outline params stay in #11's public API?** They're the one SFML-shaped concept the
   issue exposes publicly.
5. **Sequencing.** #36 and #12's `IAppLayout` split are the same change; #12's `Present()`
   split and the `Window` presentation/compositing extraction are also the same change.
   Doing them as separate passes means touching those functions repeatedly. Rough order
   that avoids that: **#36 → #11 → #12 (incl. `Window` decomposition) → `EditorShell` →
   `Workspace` rename → standalone D3D12 renderer.**
