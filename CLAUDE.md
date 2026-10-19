# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository. Global preferences (communication style, git-commit safety, naming/formatting conventions) live in `~/.claude/CLAUDE.md`; workflow-tooling rules (terminology, issue label taxonomy, phased-work convention) live in `~/.claude/skills/shared/workflow.md`. This file covers only what's specific to this repo.

## Project overview

Slush is a personal, hand-rolled C++ game engine (namespace `Slush`) built on vendored **SFML** (`SFML/`) for windowing/graphics/audio and vendored **Dear ImGui** (`Solution/Engine/imgui/`) for editor tooling. It's a single Visual Studio solution (`Solution/Solution.sln`) containing:

- `Solution/Engine/` — shared engine code (windowing, input, rendering, asset system, ImGui-based editor dockables)
- `Solution/Framework/` — low-level, STL-avoiding utility library (containers, math, string/hashing, file I/O, assertions, unit tests) — all types prefixed `FW_`
- `Solution/ActionGame/`, `Solution/BossMonster/`, `Solution/TopDownGame/` — separate game executables built on Engine + Framework

Authored content lives in the top-level `Assets/` folder, split into **Asset Repositories** (`Assets/Engine/`, `Assets/ActionGame/`, ...; see `terminology.md`). Each game's `main.cpp` mounts its own with `Slush::AssetRepository::Mount("<Game>")`, Engine is mounted last automatically, and lookups return the first Asset Repository that has the file. `Workbed/<Game>/` holds only the exe, build output and the `Debug/` folder. All path resolution goes through `Slush::AssetRepository` (`Solution/Engine/Core/AssetRepository.h`).

## Build

Windows-only, no CMake. Build only through the repo-root `build.bat`:

```
./build.bat [Debug|Release] [rebuild]    # Git Bash
.\build.bat [Debug|Release] [rebuild]    # PowerShell
```

Defaults are Debug and an incremental build; keywords are case-insensitive and can appear in any order. The script finds MSBuild via `vswhere`, falling back to PATH, and always builds all three games for x86 (the vendored SFML libraries are x86-only; see #22 for x64 support).

Use `rebuild` when switching between Debug and Release: the projects share library output paths under `Workbed\`, so an incremental build can otherwise link libraries from the previous configuration.

The script avoids Git Bash's `/`-switch path rewriting and builds the whole solution so `$(SolutionDir)` resolves correctly.

The console prints each distinct error in full, a warning count, and a final pass/fail line; full warnings and normal build output go to `Build_Output\build.log`. The exit code is zero on success and non-zero on failure.

`verify.bat [Debug|Release] [rebuild] [<Game> <scenario-path>]... [-timeout <s>]` builds and verifies the games through the PowerShell companion `verify.ps1`; see Testing below.

Building inside a git worktree (e.g. `/implement-issue`'s per-issue worktrees) doesn't collide with the primary checkout: `Directory.Build.props`'s `RepoRoot` resolves per-checkout, so each worktree gets its own `Workbed\`/`Build_Output\`.

The script disables MSBuild worker-node reuse. An interrupted build can still leave `MSBuild.exe` or `mspdbsrv.exe` (the shared PDB-writing server) holding a worktree open. If `git worktree remove` fails with a file-in-use/access-denied error, find processes whose command line/module path references the worktree's absolute path and terminate those specifically — don't reach for `git worktree remove --force` or manually delete the directory. If it still won't clear, stop and report rather than forcing it.

*(Optional, recommended if a lingering process or dirtied runtime file is a recurring problem)*
Worktree-blocking processes: ActionGame.exe, BossMonster.exe, TopDownGame.exe, MSBuild.exe, mspdbsrv.exe
Worktree-discardable paths: Workbed/*/Debug/DebugSettings.sdebug, Workbed/*/Debug/*

## Code style

These conventions differ from typical C++ defaults — follow them in this codebase (naming/brace/indentation conventions are global, see `~/.claude/CLAUDE.md`):

- Framework-library types are prefixed `FW_` (`FW_Vector2`, `FW_GrowingArray`, `FW_Intersection`) instead of being namespaced under a nested namespace — some are `namespace FW_Xxx { }` blocks, others are `FW_Xxx`-prefixed template classes
- Engine/game code uses a real namespace: `Slush::`
- `#pragma once` for header guards, never `#ifndef`
- Framework code prefers custom containers (`FW_GrowingArray`, `FW_String`) over STL equivalents; `std::string`/STL do show up occasionally in Engine/game code. New code should never user STL-functionality without direct user approval.
- Assertions use the `FW_ASSERT` macro (`Solution/Framework/FW_Assert.h`), not `<cassert>`
- Avoid bare global functions for utilities that need to be called from more than one `.cpp` — wrap them in a namespace instead (e.g. `CollisionUtils::GetNames()` in `PhysicsComponent.h`, not a free `GetCollisionFlagNames()`)
- An enum/helper-functions pair that's conceptually owned by a single component (e.g. `CollisionFlag` living in `PhysicsComponent.h/.cpp`) should stay colocated with that component rather than being pulled into its own file. Reserve a dedicated file (like `EntityType.h/.cpp`) for types that are genuinely shared by many independent components with no single owner
- Prefer exposing a getter on the owning `Component` (e.g. `PhysicsComponent::GetCollisionFlag()`) over having other components call `EntityPrefab::GetComponentData<T>()` themselves
- `FW_StaticArray<Type, Size>`'s default constructor does **not** zero-initialize elements — if a `Data` struct has one and needs a known default (e.g. all-`false`), give `Data` an explicit constructor that calls `.Fill(...)`
- `AssetReference<T>::ResolveDependency()` logs resolution failures centrally (naming the holding asset via `DependencyTracker::GetCurrentAssetName()`). Call sites are bare `if (T* p = ref.Get()) …` — no per-call-site fallback, no per-call-site logging, and never spawn a placeholder empty entity
- Exception: use a hard `FW_ASSERT` where the asset is a genuine invariant rather than a recoverable absence (e.g. `Level::Restart()`'s player spawn)
- `AssetReference<T>` deliberately exposes no mutable string accessor — editor UI renders it read-only (`ImGui::Text`), not `ImGui::InputText`
- Hoist a singleton's `Foo::GetInstance()` into a local reference before repeated or looped calls rather than re-invoking it per call (`AssetRegistry`, `ComponentRegistry`, `DependencyTracker`, `Engine` all use this singleton shape)
- UI look comes from the game's `UITheme`, owned by its Globals. UI call sites pass no fonts, text sizes or hex colours and own no `UIElementStyle` members for look; size and arrangement stay at the call site.

## Testing

There's no third-party test framework (no GoogleTest/Catch2). `Solution/Framework/FW_UnitTestSuite.h/.cpp` is a small shared runner that records every failed check and keeps going. `Slush::UnitTests::Run` (`Solution/Engine/Core/UnitTests.h/.cpp`) runs every suite (Framework, then a game-specific callback passed in by the game's `main.cpp`) between `CommandLineArgs::Parse` and `Engine::Initialize`, and writes `Workbed/<Game>/Debug/unittest_results.txt`. Line 1 is always `UNITTEST_RESULT: PASS (0 failed / M checks, T tests)` or `UNITTEST_RESULT: FAIL (N failed / M checks, T tests)`; each failure is then one line, `FAIL <Suite>::<Test>  <file>(<line>): <expression> -- <message>`. Passing tests aren't listed. A failed check never crashes: every test still runs, and there's one debug break at the end only if a debugger is attached.

A suite starts with `FW_UnitTestSuite::BeginSuite("Name")` and runs its tests via `FW_RUN_TEST(TestName)`. Inside a test use `FW_TEST_CHECK(expr, "msg")` (record and continue) or `FW_TEST_REQUIRE(expr, "msg")` (record and `return` from the test) — `REQUIRE` only where a failure makes the rest of the test unsafe or meaningless (e.g. a count check before indexing). `FW_ASSERT` stays for non-test code.

There's no option to skip tests: the whole suite runs on every launch of every game, so tests must stay fast. `<Game>.exe -runtestsonly` runs every suite, writes the report and exits before `Engine::Initialize` (exit code 0 on pass, 1 on fail) — the cheap verification path, no window or assets needed. Tests run before the Engine is initialized, so they can't use the logger, window or asset system. A test needing scratch files uses `Slush::AssetRepository::GetDebugFilePath(...)` (creates `Workbed/<Game>/Debug/` on first use); `AssetParser`/`FW_FileParser` take absolute paths, so pass them the resolved path.

`DataAsset` types carry a version number (`DEFINE_ASSET`'s last argument) and auto-resave on `Load()` if the file's `version` field is behind (see `Solution/Engine/Core/Assets/DataAsset.cpp`). When a change triggers this kind of resave across `Assets/` files, diff more than one resaved file in full — a resave normalizes the *entire* file to match current parsing code, so it can surface unrelated latent drift (stale unused fields, newly-required fields) that's easy to miss if you only check that the version number changed.

For changes that could affect actual gameplay (entity spawning, component construction, weapon/projectile logic) rather than just asset loading, verify by getting into real gameplay, not just a clean asset-load log. `ActionGame`'s `SkipStartScreen` debug flag (`Workbed/ActionGame/Debug/DebugSettings.sdebug`, also toggleable via the in-editor "Debug Settings" dockable) skips the main menu and character-selection screen and drops straight into a level — enemies spawn, entities update, projectiles fire, from a cold launch. Stronger evidence than log output alone; consider a mid-run screenshot to confirm visually. Toggle it back off afterward — it's an opt-in debug convenience, not default behavior. The `SkipStartScreen` setting is `ActionGame`-specific (`MainMenuState`/`CharacterSelectionState`).

Use `verify.bat` for repeatable verification: with no scenarios it builds and runs all three games' unit tests. Add any number of `<Game> <scenario-path>` pairs to run headless debug scripts afterward, for example `verify.bat Debug TopDownGame path/to/menu.script ActionGame path/to/action.script`. Relative scenario paths resolve from the script's repository root, including inside worktrees. Scenario timeout defaults to 120 wall-clock seconds; override with `-timeout <s>`. A timeout stops only the launched process, by its process handle/PID. Unit-test processes also have a 120-second limit. Build failure stops immediately. Test failures, scenario nonzero exits, timeouts, missing reports/logs, or scenario log `[ERROR]` lines produce `VERIFY FAILED` and exit 1. Warnings are counted without failing. Each scenario gets a distinct `Debug/verify_<game>[_<n>].txt` log. Success ends with `VERIFY PASSED` and exit 0.

Every Slush game is a Windows-subsystem executable, with no console window. For a direct headless run, use `<Game>.exe -hidewindow -debugscript <path> -logname <name>`. The startup script path is absolute or relative to the game's working directory. For live poking, add `-usedebuginput`: each frame appends lines from `Workbed/<Game>/Debug/debug_input.txt` to the same queue, then truncates the file. Startup lines run first. Both channels use debug commands; bare integer lines are no longer supported.

Commands run in order each frame until a wait blocks or the queue is empty. Double quotes group arguments containing spaces; blank lines and lines whose first non-space character is `#` are ignored. A failing line logs an error and the script continues, allowing a trailing `quit` to run. Any script failure makes the eventual process exit code 1; otherwise it is 0. A missing startup script also fails but leaves the game running.

- `key <name>` injects a one-frame press using case-insensitive KeyCode names without the leading underscore (`F10`, `Esc`, `1`, `space`, `hyphen`). ImGui keyboard capture fails only that line.
- `wait <n>s` waits engine seconds (summed engine delta); `wait <n>f` waits frames. Fractions are allowed; missing units, negative and invalid values fail.
- `waitfor <trigger> [timeout]` waits for a trigger posted since the last satisfied `waitfor`, or startup. Satisfaction clears the entire trigger record. Timeout uses engine seconds, defaults to `10s`, and fails the line before continuing.
- `click "<label>"` clicks a UIBuilder button by its identifier in that frame, independent of window size, DPI or docking layout. The first matching builder to finish consumes it. Disabled and missing buttons fail with distinct errors. Only one click is allowed per frame; a second fails and is ignored. Use a wait between clicks when needed.
- `screenshot [name]` captures the frame presented at the end of that frame. A named capture overwrites `Debug/<name>.png` without rotation. No name, or `key F10`, writes `Debug/screenshot.png` and rotates the previous file to `screenshot_previous.png`. Names use `[A-Za-z0-9_-]`, with no extension; invalid names fail.
- `quit` follows normal shutdown, warns and discards unsaved asset changes, and flushes the log. End headless scripts with `quit`, never kill the process. The verification wrapper's timeout is a fallback for a stuck or incomplete script.

Game code posts triggers through `DebugScriptRunner::PostTrigger`. StateStack automatically posts `gamestate.<name>.active` after start/resume and `.inactive` before cover/end. Every trigger appears in the log as `[Trigger] <name>`; find available names there. Activation describes lifecycle, not rendering readiness: include an explicit wait before startup screenshots (follow-up #138 tracks a better readiness signal).

Example TopDownGame menu-to-level scenario:

```text
# Select the first playable level by its displayed button label
waitfor gamestate.mainmenu.active
click Start
waitfor gamestate.levelselect.active
click "1  level_main"
waitfor gamestate.level.active
wait 2s
screenshot level_loaded
quit
```

TopDownGame starts with Start and Quit buttons; Start opens level selection. `-level <LevelData asset name>` enters a level with the menu underneath. A missing value warns and shows the menu; an unknown asset asserts. `-levelselect` opens selection directly, and is ignored with `-level` or `-towertest`. `-towertest` skips the menu and ignores `-level`. `-layout <name>` starts in the layout whose `IAppLayout::GetName()` matches (e.g. `LevelEditor`); a missing or unknown name warns and keeps the default Game layout. The Level Editor's level runs in edit mode: no waves and no win/lose result. It opens `-editlevel <LevelData asset name>` if given (an unknown asset asserts, a missing value warns), otherwise the last level it edited (remembered in the untracked `Debug/leveleditor_lastlevel.txt`), otherwise `level_main`. GameOver and LevelSelect use label clicks; their test-only number-key shortcuts are removed. ActionGame's corresponding script clicks `"Start Game"`, waits for `gamestate.characterselection.active`, clicks a character label such as `"Bow Man"`, then waits for `gamestate.level.active`.

Label clicks are available for automated verification. Real mouse positions, movement and drags remain manual checks: describe them for the user rather than scripting fragile screen coordinates.

Manual-test gate: launch `Workbed/<Game>/<Game>.exe` directly, no flags — real menu → character-select → gameplay flow, not the headless path above. `<Game>` is `ActionGame`/`BossMonster`/`TopDownGame`, inferred from the issue's `project:` label (`project:actiongame` → `ActionGame`; ask which game(s) for `project:engine` or anything ambiguous).
Manual-test folder: Workbed
Manual-test launch: project:actiongame -> Workbed/ActionGame/ActionGame.exe
Manual-test launch: project:topdowngame -> Workbed/TopDownGame/TopDownGame.exe

Use `-logname <name>` with verification launches to write `Debug/<name>.txt` instead of `debuglog.txt`, overwriting any existing log of that name. Names must be non-empty and contain only `[A-Za-z0-9_-]`; missing or invalid values fall back to `debuglog.txt` with a warning. With no flag, logging uses `debuglog.txt` as before. Agents running multiple verification launches should name each run rather than copy `debuglog.txt` around. All `Workbed/*/Debug/` files, including logs, debug input and screenshots, are gitignored, except the tracked `Debug/DebugSettings.sdebug`.

## Repo etiquette

Base/merge branch: `main`. Solo project, but `/implement-issue`/`/implement-issue-orchestrated` use a PR flow rather than direct-commit: work happens on a transient `issue-<N>` branch inside a git worktree, then gets pushed and opened as a GitHub PR (`Closes #<N>` in the body) instead of merged locally. Merging itself is never automated — it happens only when you click Merge on GitHub, which is the actual review gate — and the worktree/branch are torn down once the PR is open.
PR-flow token: <AGENT_GH_TOKEN>

Agent summaries, findings, PR/issue bodies and other scratch output belong in the session scratchpad or a temp file outside every checkout and worktree. Delete temp files right after use (for example, after `gh --body-file`), and post summaries and findings to the GitHub issue or PR where they are read. Never redirect build or run output into the repo (`> something.log`); use `build.bat`'s `Build_Output/build.log` and `-logname` instead.

## Issue tracking

Work is tracked on GitHub Issues at `nian0601/Slush` (migrated from Trello). Valid `project:` label values: `project:engine` (Slush Engine/Framework code), `project:actiongame` (ActionGame-specific code), `project:topdowngame` (TopDownGame-specific code), `project:tooling` (Claude Code skills, CLAUDE.md, repo workflow tooling). Revisit if this needs to be more granular (e.g. per-game, as BossMonster becomes active) or consolidated.

## Autonomy

Gates are defined in `~/.claude/skills/shared/workflow.md` section 7. `G1` is granted because nothing lands unseen: every change still goes through a GitHub PR whose Merge click is the actual review gate (`## Repo etiquette`), and every call made under `G1` is filed as an `agent-flagged` issue for later review.

Granted: **`G1`** (judgment calls).
Withheld: **`G2`** (merge), **`G3`** (close) — moot under the PR flow: merging stays the user's Merge click, and the PR's `Closes #<N>` closes the issue. Withheld rather than granted so dropping the PR flow would never quietly start merging. **`G4`** — every plan is reviewed before it becomes an issue.

Each always-halt below fires only when the issue's phases don't call for it; a plan that does is caught earlier, at plan review (see below).

Always halt regardless:

- New STL usage (see `## Code style`; `G1` doesn't get around the approval rule).
- A change that requires mass-resaving `Assets/` files, i.e. existing files no longer load correctly unless rewritten. A `DataAsset` version bump (`DEFINE_ASSET`'s last argument) on its own isn't a halt: `DataAsset::Load()` passes the file's version to `OnParse`, so old files can keep loading.
- Build-config changes beyond adding/removing source files: include paths, linked libraries, compiler/linker flags, configurations/platforms, `Directory.Build.props`, `build.bat`.
- Adding, removing or upgrading third-party code (`SFML/`, `Solution/Engine/imgui/`, or a new library).

Plan review escalates:

- A plan that calls for new STL usage.
- A plan that calls for build-config changes beyond adding/removing source files (include paths, linked libraries, compiler/linker flags, configurations/platforms, `Directory.Build.props`, `build.bat`).
- A plan that calls for adding, removing or upgrading third-party code (`SFML/`, `Solution/Engine/imgui/`, or a new library).

A plan requiring a mass resave of `Assets/` is already escalated by baseline item 2 (stored schema / data migration), so it isn't repeated here.

The non-delegable floor in `workflow.md` section 7 applies on top of these and cannot be waived here. No grant skips the manual-test gate (`## Testing`).
