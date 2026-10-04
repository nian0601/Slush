# Slush codebase review — Claude (2026-10-04)

Read-only review of `main` @ `8821f27`. Four parallel investigations: Framework + engine core, rendering/editor/build, the three games, and how well the project suits AI agents. About 10 of the highest-impact claims were spot-checked against the code (marked ✔). Unmarked items come from the subagents and were not individually re-verified. No code or data was changed, and no earlier reports were read.

## Short version

The design is mostly sound. Worth keeping:
- entity handles through proxies, with deferred add/remove
- a single `OnParse` for both reading and writing
- versioned assets
- per-prefab component data
- a clean Framework → Engine dependency direction

The weak spots are lower down:
- **Framework containers have real bugs, with almost no tests.**
- **Failures are silent:** `FW_ASSERT` throws away its message.
- **Runs can't be repeated:** frame timing is variable and RNG is the global `rand()`.
- **Weak verification loop:** agents check their work by launching the game and reading the log. There's no exit code, no CI and no real headless mode.

---

## 1. Bugs worth fixing now (each is small)

### Framework

| | Problem | Where |
|---|---|---|
| ✔ | `FW_RandInt` uses `&` instead of `%`, so many values never come up (`RandInt(0,2)` never returns 1). Used for enemy picks, drops and upgrades. | `FW_Math.h:35` |
| ✔ | Integer hashing passes `len=1` to Murmur, so only the low byte is hashed (at most 256 distinct hashes). `FW_Hashmap` also has a fixed 67 buckets and never rehashes. | `FW_Hashing.h:14,22,30,38`; `FW_Hashmap.h:284` |
| ✔ | `FW_String +=` asserts on overflow *before* the resize that would have handled it. Asserts are on by default (`FW_STRING_ASSERTS`), so a log message over ~240 chars crashes. The same pattern appears at lines 140/158/176/193/207. | `FW_String.h:117-123` |
| | `FW_String ==` compares only the hash, and `Clear()`/`UpdateStringFromImGUI` don't rehash, so `FW_String() != FW_String("")`. `Length()` returns count−1. `Find`/`RFind` index a jump table with a signed `char` (UTF-8 input gives a negative index), and `RFind` reads one past the end. The `char` constructor doesn't null-terminate. Every string allocates 255 bytes. | `FW_String.h` |
| | `FW_GrowingArray`: self-assignment frees, then reads its own buffer. `arr.Add(arr[0])` at full capacity reads freed memory. `DeleteLast`/`RemoveLast` assert `>= 0` instead of `> 0`. `Reserve()` actually resizes. | `FW_GrowingArray.h:118-123,198-206,290,355` |
| ✔ | Bounds checking is only enabled through `FW_Includes.h`, so the same template is compiled with and without checks (an ODR violation). | `FW_Includes.h:3` |
| | `delete`/`new[]` mismatch. | `FW_FileProcessor.cpp:55`, `FW_FileSystem.h:23` |

### Engine core

| | Problem | Where |
|---|---|---|
| ✔ | `~EntityManager` deletes proxies before entities, so entity handle destructors touch freed memory. Happens on every level teardown. Entities still in the add queue leak. | `EntityManager.cpp:16-17` |
| | `EntityHandle::operator=` increments the ref count without a null check, so assigning an empty handle crashes. | `EntityHandle.h:56` |
| ✔ | `FW_ASSERT("literal")` is always true, so a missing asset storage silently returns storage of the wrong type. | `AssetStorage.cpp:56` |
| | A failed or empty parse leaves the version at 0, which triggers an auto-resave that overwrites the file with defaults. Floats are written as `%.3f`, so every resave rounds them. Nothing guards against a file version newer than the code's. | `DataAsset.cpp:12-23` |
| | Physics: manifolds are cleared once per frame but appended every substep, so they are resolved twice when frames drop. A `static` accumulator is shared across worlds and restarts. `PhysicsObject` is deleted by both `~PhysicsComponent` and `PhysicsWorld::DeleteAllObjects`. | `PhysicsWorld.cpp:159,199-226,267-271` |
| | Copied assets keep the source asset's file path, and their references are never resolved. | `AssetStorage.h:103-105`, `Asset.cpp:14` |
| | Placeholder empty entity spawned when a prefab is missing, contrary to `CLAUDE.md`. | `EntityManager.cpp:50-51` |
| | StateStack: `myMainIndex`/`mySubIndex` are not updated on pop, and `PopMainState` resumes substates that are about to be popped. | `StateStack.cpp:27,85-86` |
| | Logging before `Initialize` or after shutdown dereferences a null logger. Registry `Destroy()`s are never called. | `Log.h:93-96` |
| | `GetModuleFileNameA` writes into a 128-byte buffer, so a deep worktree path gets truncated and the data folder is wrong, with no error. | `Engine.cpp:53`, `FW_UnitTestSuite.cpp:29` |
| | `QuadPart * 1000000 / freq` overflows int64 after ~10 days of system uptime. | `Time.cpp:21,54` |

### Rendering, input, editor

| | Problem | Where |
|---|---|---|
| ✔ | Input states stop aging while ImGui has keyboard focus, so `WasKeyPressed` keeps returning true. Input is read even when the window is unfocused. `IsKeyDown` is false on the frame the key is pressed. | `Engine.cpp:115`, `Input.cpp:111,137`, `Input.h:74` |
| | Mouse is mapped to window space, but the UI is laid out in 1920×1080 buffer space, so clicks drift after any resize. | `Engine.cpp:124`, `UIBuilder.cpp:551`, `Window.cpp:135` |
| | Textures/fonts that fail to load are dereferenced anyway. Reloading leaks the old SFML object. `Font` is copyable with a raw owning pointer. | `Renderer.cpp:269`, `Text.cpp:18`, `Texture.cpp:18-22`, `Font.h` |
| | BossMonster likely asserts at startup: `Data/NotoSans.ttf` is hard-coded and that game doesn't have it. | `Engine.cpp:75-88` |
| | `SetAppLayout` deletes layouts it may not own, and `~Window` leaks the app layout. ActionGame's 1/2 hotkeys swap layouts, bypassing the unsaved-changes check. | `Window.cpp:44-52,286`, `ActionGame/main.cpp:66-85` |
| | A closable dockable is deleted without checking for unsaved changes, and can leave `myCloseRequestBlocker` dangling. | `IAppLayout.cpp:33-37,74` |
| | Editor "Close Without Saving" leaves the modified in-memory asset live. | `AssetEditorDockable.cpp:118-122` |
| | No frame cap or vsync: the loop runs uncapped at 100% CPU/GPU. | `Engine.cpp:177-187` |

### Games

| | Problem | Where |
|---|---|---|
| ✔ | ActionGame never frees `EntityManager`/`PhysicsWorld`, so they leak on every restart. | `ActionGame/StateStack/LevelState.cpp:20-38` |
| ✔ | Enemy spawn clearance does nothing: the `continue` is inside the inner loop. | `ActionGame/Level/Level.cpp:143-147` |
| | `TopDownGameGlobals::SetLevel(this)` is never cleared, so the pointer will dangle once level transitions (#88/#89) land. TopDown already runs two `Level`s (game + editor). | `TopDownGame/Level/Level.cpp:135` |
| | Character selection has no effect: `myCharacterEntityPrefab` is never used. | `CharacterInfo.h:27`, `Level.cpp:86` |
| | Q is bound twice (weapon upgrade + spawn). `myEnemyWaves[0]` is read without an empty check. `myDamageAnimation` is uninitialized. | `Level.cpp:65,82`; `PlayerControllerComponent.cpp:65-70`; `HealthComponent.h:50` |
| | Several state pushes can happen in one frame (upgrade + game over + pause). | `LevelState.cpp:52-61` |
| | ActionGame targeting can pick entities that are about to be removed. | `TargetingComponent.cpp:37-45`, `NPCControllerComponent.cpp:84` |

---

## 2. Structural changes (stability, extensibility, modularity)

1. **Make asserts useful.** Log the message, file and line, flush the log, then break. Separate hard invariants from checks the tests can count. Use errors, not asserts, for I/O failures such as a failed screenshot save (`Window.cpp:168,190`). This is the cheapest change with the biggest payoff.
2. **Test the Framework properly.** `FW_String`, `FW_GrowingArray` and `FW_Hashmap` have zero tests. The suite currently contains only `TestFileProcessor`. Tests would have caught most of section 1's Framework bugs.
3. **Add an engine `World`/`Scene`.** It would own the `EntityManager` and optional `PhysicsWorld`, and have a `Tick(dt)` that runs the phases and dispatches collisions. That would:
   - remove the frame loop copied into each game (`ActionGame/LevelState.cpp:47-96`, `TopDownGame/Level.cpp:155-156`),
   - remove the per-game `*Globals` singletons and the leaks,
   - let components reach their context through `myEntity.myEntityManager` instead of globals.
4. **Invert component registration.** The engine declares `EntityManager::RegisterComponents()` and each game defines it, so the contract is only enforced at link time. Instead, the engine registers its own components and each game passes a callback. Assign type IDs explicitly at registration instead of lazily in first-use order (`AssetStorage.h:217-219`). Replace the hard-coded `32` with `MAX_COMPONENTS` (`Entity.h:45`, `EntityPrefab.h:47`).
5. **Lift the duplicated game shells into the engine:**
   - `GameLayout`, which becomes a `StateStackLayout`
   - `main()` boilerplate, which becomes `RunApp<App>`
   - `*Globals` font setup
   - `HealthComponent`
   - UI style presets (`SetPadding(16,16)` appears 16 times)
6. **Replace hard-coded asset names with `AssetReference` fields.** Strings like `"Blink"`, `"Dash"`, `"SpriteSheet"`, `"level_main"`, `"Wall"`, `"Enemy_Normal"` and `"Tower_Basic"` fail silently on rename and are invisible to `DependencyTracker`.
7. **Make dying entities explicit.** Have `IsAlive()` (or `Get()`) treat entities marked for removal as dead, rather than every caller checking `myIsMarkedForRemoval` by hand. While there, use the entity's own proxy directly instead of scanning every proxy in `EndFrame`.
8. **Add spatial and component queries.** `GetAllEntities` copies handles for every targeter every frame, which is O(N²). That will hurt a tower defense game. Add `ForEachWithComponent<T>` or a radius query.
9. **Separate the editor from the runtime.**
   - ImGui is in the engine's precompiled header.
   - Assets and components carry `BuildUI`, and `Animation` holds editor tool data.
   - The game view is itself a dockable.
   - Short term: reload from disk when an edit is discarded.
   - Long term: add a `SLUSH_EDITOR` define, move the inspectors out of runtime types, and have `IApp` own game update and render.
10. **Make the renderer extensible.** One fat `RenderCommand` switch, one draw call per primitive, 1920×1080 hard-coded in four places, and a full-buffer copy every frame for fade. Add a sort key (layer, texture), batch per texture, make the resolution a config value, and copy for fade only when it starts.
11. **Clean up the build:**
    - All projects at `/W4 /WX` (the games are at `/W3` with no warnings-as-errors).
    - Separate Debug and Release output names; both write the same `.lib`/`.exe` into `Workbed`.
    - Finish x64 (#22) or remove the broken x64 configs (Engine x64 is set to `Application`).
    - Fix the defines: add `SFML_STATIC` to the games, and remove `_CONSOLE` from TopDown.
    - Gitignore `Build_Output/`, `ImGUILayouts/*.ini`, `DebugSettings.sdebug` and `.vs/`, and ship `.default` copies.
12. **BossMonster:** port it to the asset/state system or archive it, rather than keeping engine APIs (`IApp::Render`, the legacy `#key value` format) alive for it. Remove the junk data files (`Enter File Name.boss`, `testing_for_realies.boss`).

---

## 3. If rebuilding for AI-heavy work

Ranked by payoff.

1. **Deterministic simulation core with no SFML or ImGui dependency.** Fixed timestep, seeded RNG per system (PCG/xorshift in `FW_`), and rendering that only reads simulation state. This is the biggest gain: "load level, run N ticks with scripted input, assert state" becomes a normal test. It also allows replays and golden-file tests, and the core builds on Linux, where cloud and sandboxed agents run.
2. **Separate test executable** (console subsystem). Named tests, file:line, messages, a pass/fail count, a name filter, and a non-zero exit code. Tests stop running on every game launch.
3. **Real headless mode.**
   - Example: `--headless --frames N --script in.txt --dump-state out.json`.
   - The script is frame-stamped and uses key names (`120 KEY F1`), with optional command lines (`place_tower Basic 700 700`). The process exits with a code.
   - This replaces today's loop: launch, wait, write integers to `debug_input.txt`, wait, grep the log, `taskkill`.
   - Today's `-hidewindow` only hides the window; OpenGL, ImGui and rendering still run.
4. **Structured logging.** One line per entry with frame, severity, channel and source location, written to stdout and file, and flushed on every error or assert. Today the log flushes every 3 s or every 25 entries (`Log.cpp:146-149`), so a crash or kill loses exactly the lines that matter.
5. **State introspection.** Dump entities, components, resources and wave state as JSON, so agents read state as text instead of interpreting screenshots.
6. **CMake with presets** (or at minimum one `build.ps1`). That gives one canonical build command and per-target builds, and removes the MSBuild and worktree process-hygiene rules in `CLAUDE.md`. Fetch SFML through vcpkg or a package instead of committing x86-only `.lib`s.
7. **Data with a schema, plus an `assetcheck` command line tool** that parses every asset, resolves references, reports unknown and missing keys, and never resaves.
   - Store colors as hex, not packed signed ints (`color -256`).
   - Generate navmeshes from authored shapes rather than hand-editing an 18 KB vertex dump.
   - Today, key typos persist forever, e.g. `baseprojectilcount` in `wpn_fireball.weapondata`.
8. **Editor actions as named commands that ImGui calls.** Agents could then do level and navmesh editing, which today is mouse-only and out of scope for them, and undo/redo comes almost for free.
9. **Enforce the style rules with tools:** `.clang-format` (Allman, tabs), `.editorconfig`, a clang-tidy naming rule for `my`/`a`/`an`/PascalCase, and a banned-include check. Those conventions cost agents almost nothing once a tool enforces them.
10. **Rethink the STL ban.** It costs more than the naming rules: you own and must test every container yourself, which is where the bugs in section 1 come from. Allow the STL in tests and tools at least, or keep the ban but cover the `FW_` containers thoroughly.
11. **CI** (GitHub Actions `windows-latest`, plus Linux for the simulation core): build, tests, asset check, a headless smoke run per game, and a format check. The review step then reads an objective result instead of trusting the agent's own claims.
12. **Keep runtime-mutated and per-machine files out of git.** That removes the "toggle `SkipStartScreen` back off" rule and the list of paths that are safe to discard.

---

## 4. Getting most of that without a rewrite (each about 1–2 days)

1. Rewrite `FW_ASSERT` to log the message, file and line; flush the log on every error; add a frame number to each log line; fix `FW_RandInt`; add tests for the containers.
2. Add a `Tests` console project to the `.sln`. It links Framework plus the pure-logic files (`WaveScaling`, `Navmesh`), runs the existing suites with a counting `CHECK`, and returns an exit code. Make the tests that run at game startup Debug-only, or drop them.
3. Add `-frames N` and a process exit code (non-zero if any ERROR/FAIL was logged). Make debug input frame-stamped with key names. Turn `-towertest` into a script that ends itself with pass/fail, and later into a real `TowerPlacementTestSuite`. `TryPlaceTower` already returns a result enum.
4. Add `-fixeddt` (`GetDelta()` returns 1/60) and a `-seed` flag.
5. Add `scripts/build.ps1` and `scripts/test.ps1` that wrap the canonical MSBuild command line, and allowlist only those.
6. Add `.clang-format` and `.editorconfig`, set `/W4 /WX` on the game projects, and add a grep-based check for STL includes in Framework. Then remove the matching prose from `CLAUDE.md`.
7. Add an `-assetcheck` mode. `LoadAllAssets` and `DependencyTracker` already exist, so this is cheap.
8. Add a Windows CI workflow running steps 2, 3, 5 and 7.
9. Gitignore the files the game rewrites at runtime.
10. Later: pull Level, Navmesh and Economy logic into an SFML-free simulation library. That is the bridge to a Linux build and real headless tests.

---

## Keep these

- Framework has no dependency on Engine, ImGui or SFML.
- SFML is well contained behind forward declarations. Only three game files touch `sf::`, all of them for `ImGui::Image`.
- The game sees backend-neutral `Input::KeyCode` and `Renderer::RenderX` APIs.
- Per-prefab `BaseData` vs per-entity `Component`, with one line of registration per component.
- A single `OnParse` for read and write. Per-asset and per-component versioning with auto-upgrade.
- Two-pass `AssetReference` resolution, central failure logging, and `DependencyTracker`.
- `EntityHandle` weak handles through proxies, with deferred add/remove. Fix the bugs, keep the design.
- TopDown's `Level` owning its `EntityManager` by value. Explicit result enums (`PlaceTowerResult`, `LevelResult`).
- Pure logic namespaces that take randomness as input (`WaveScaling`), tested at startup. This is the model to spread.
- `GamePhysicsData` hooks, which let a game define collision flags without the engine knowing about them.
- New asset types and dockables show up in the editor automatically.
- `Engine::Run` split into named phases, which makes a fixed-step loop easy to add.
- `RepoRoot` resolving per worktree. Headless hooks (`-hidewindow`, `-usedebuginput`, F10 screenshots).

---

## Process note

`.claude/settings.local.json` allows `Bash(git commit *)` and `Bash(git *)`, while the global `CLAUDE.md` says never to commit without an explicit ask. Prose and enforcement disagree here. Left unchanged.
