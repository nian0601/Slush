# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Communication style

Prefer short, plain language over grammatical polish. Fragments and dropped words are fine if the meaning lands — brevity wins when it conflicts with grammar. Applies to explanations, status updates, and commit messages; code comments follow the separate (also terse) rules under Code style.

## Terminology

@.claude/terminology.md

Grilling and planning sessions (`grilling`, `plan-issue`, `plan-issue-with-grill`) actively watch for terminology drift — conflicting uses of the same word, unclear language, or a concept getting re-described repeatedly without ever being given a specific name — and flag it; see those skills for the exact behavior. Any other session should still check this glossary before relying on a term it defines.

## Project overview

Slush is a personal, hand-rolled C++ game engine (namespace `Slush`) built on vendored **SFML** (`SFML/`) for windowing/graphics/audio and vendored **Dear ImGui** (`Solution/Engine/imgui/`) for editor tooling. It's a single Visual Studio solution (`Solution/Solution.sln`) containing:

- `Solution/Engine/` — shared engine code (windowing, input, rendering, asset system, ImGui-based editor dockables)
- `Solution/Framework/` — low-level, STL-avoiding utility library (containers, math, string/hashing, file I/O, assertions, unit tests) — all types prefixed `FW_`
- `Solution/ActionGame/`, `Solution/BossMonster/`, `Solution/TopDownGame/` — separate game executables built on Engine + Framework

## Build

Windows-only, no CMake. Use MSBuild directly (find its path via `vswhere` if not on PATH):

```
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" Solution\Solution.sln /p:Configuration=Debug /p:Platform=x86 /m
```

Use `Platform=x86` (not `x64`) — the vendored SFML libs in `SFML/lib/` are only built for x86, so `x64` configs don't fully build. `x86` builds all three game executables cleanly. (x64 support is incomplete beyond the libs too: `Engine.vcxproj`'s x64 configs are missing PCH settings, the `SFML_STATIC` define, and library dependencies — stick to `x86` until that's all ported over.)

Adding new files through Visual Studio's Solution Explorer ("Add > New Item"/"Existing Item") automatically updates the relevant `.vcxproj`, so no extra step is needed for new files to show up in CLI builds.

Always build via `Solution.sln`, not by pointing MSBuild at an individual `.vcxproj` — project-level `IncludePath`s rely on `$(SolutionDir)`, which isn't resolved correctly when a project is built standalone (e.g. `Engine.vcxproj` alone fails with `Cannot open include file: 'FW_Includes.h'`).

## Code style

These conventions differ from typical C++ defaults — follow them in this codebase:

- Member variables: `my` + PascalCase (`myVertices`, `myHasAnchor`). The `my` prefix is reserved for member variables only — local variables declared inside a function body must not use it.
- Function parameters: `a`/`an` + PascalCase (`aVertex`, `anExpression`)
- Types/functions/methods: PascalCase (`GetVertex`, `CreateEdge`)
- Framework-library types are prefixed `FW_` (`FW_Vector2`, `FW_GrowingArray`, `FW_Intersection`) instead of being namespaced under a nested namespace — some are `namespace FW_Xxx { }` blocks, others are `FW_Xxx`-prefixed template classes
- Engine/game code uses a real namespace: `Slush::`
- Braces: Allman style (opening brace on its own line)
- Indentation: tabs, not spaces
- `#pragma once` for header guards, never `#ifndef`
- Framework code prefers custom containers (`FW_GrowingArray`, `FW_String`) over STL equivalents; `std::string`/STL do show up occasionally in Engine/game code
- Assertions use the `FW_ASSERT` macro (`Solution/Framework/FW_Assert.h`), not `<cassert>`
- Avoid bare global functions for utilities that need to be called from more than one `.cpp` — wrap them in a namespace instead (e.g. `CollisionUtils::GetNames()` in `PhysicsComponent.h`, not a free `GetCollisionFlagNames()`)
- An enum/helper-functions pair that's conceptually owned by a single component (e.g. `CollisionFlag` living in `PhysicsComponent.h/.cpp`) should stay colocated with that component rather than being pulled into its own file. Reserve a dedicated file (like `EntityType.h/.cpp`) for types that are genuinely shared by many independent components with no single owner
- Prefer exposing a getter on the owning `Component` (e.g. `PhysicsComponent::GetCollisionFlag()`) over having other components call `EntityPrefab::GetComponentData<T>()` themselves
- `FW_StaticArray<Type, Size>`'s default constructor does **not** zero-initialize elements — if a `Data` struct has one and needs a known default (e.g. all-`false`), give `Data` an explicit constructor that calls `.Fill(...)`

## Testing

There's no third-party test framework (no GoogleTest/Catch2). `Solution/Framework/FW_UnitTestSuite.h/.cpp` defines individual `TestXxx()` functions called from `FW_UnitTestSuite::RunTests()`, which each game's `main.cpp` invokes automatically at startup. Assertions inside tests use `FW_ASSERT`.

Note: commit messages mentioning "test" for the navmesh/intersection work (e.g. testing `FW_Intersection` against navmesh triangles) refer to **interactive/visual runtime checks** in `Navmesh.cpp`'s `Render()`/`Update()` (drag the mouse, see highlighted triangles/intersection points) — not entries in `FW_UnitTestSuite`. Don't conflate the two when asked to "add a test."

`DataAsset` types carry a version number (`DEFINE_ASSET`'s last argument) and auto-resave on `Load()` if the file's `version` field is behind (see `Solution/Engine/Core/Assets/DataAsset.cpp`). When a change triggers this kind of resave across `Workbed/` files, diff more than one resaved file in full — a resave normalizes the *entire* file to match current parsing code, so it can surface unrelated latent drift (stale unused fields, newly-required fields) that's easy to miss if you only check that the version number changed.

For changes that could affect actual gameplay (entity spawning, component construction, weapon/projectile logic, etc.) rather than just asset loading, prefer verifying by getting into real gameplay instead of just confirming a clean asset-load log. `ActionGame` has a `SkipStartScreen` debug flag (`Workbed/ActionGame/Data/DebugSettings.sdebug`, also toggleable via the in-editor "Debug Settings" dockable) that, when set, skips both the main menu and character-selection screen and drops straight into a level — enemies spawn, entities update, projectiles fire, all from a cold launch. This is much stronger evidence than log output alone; consider taking a screenshot mid-run to visually confirm. Toggle it back off afterward rather than leaving it committed as `1`, since it's meant to be an opt-in debug convenience, not the default launch behavior. This is currently an `ActionGame`-specific feature (implemented in `MainMenuState`/`CharacterSelectionState`); when starting a new game project, implement equivalent skip-to-gameplay functionality there too rather than relying on manual click-through testing.

Every Slush game launches as a `Windows`-subsystem executable (see `Directory.Build.props`), so no console window ever appears alongside the SFML one. Claude's own in-application verification should launch with `-hidewindow` for a genuinely headless run (no SFML window, no console) rather than a normal visible launch. Input should be delivered by also launching with `-usedebuginput` and writing `Input::KeyCode` integer values (see the enum in `Solution/Engine/Core/Input.h`), one per line, to `Workbed/<Game>/Data/debug/debug_input.txt` — this is polled every frame and is the only injection path that works while the window is hidden; the old approach of physically sending a keypress to a focused, visible window is no longer necessary. For visual confirmation, inject `Input::KeyCode::_F12`'s value to trigger the in-engine screenshot capability instead of an OS-level screenshot, then read `Workbed/<Game>/Data/debug/screenshot.png` (and `screenshot_previous.png`, the prior capture, for before/after comparisons). Simulating mouse movement/clicks to drive the UI (selecting list rows, dragging, clicking buttons) is still out of scope for Claude to do itself — it's slow, fragile (coordinates drift across window moves/resizes, DPI, stale docking layouts), and easy to misread. For any verification step that needs more interaction than a keypress, describe what to check and hand it to the user to click through manually rather than automating it.

## Repo etiquette

Solo project — commit directly to `main`, no branch/PR convention to follow. The one sanctioned exception: `/implement-issue` does its work on a transient `issue-<N>` branch inside a git worktree, fast-forwarded back into `main` and deleted once the issue is done — never a long-lived branch.

Claude should never run `git commit` without the user explicitly asking for it in that turn. Creating/editing/staging files does not need separate approval — only the commit itself does.

For engine-sized changes (new subsystems, cross-cutting refactors), prefer landing the work as multiple independently buildable/testable phases rather than one large commit. Each phase should result in a runnable and testable executable, dont update function-signatures or interface without updating callsites. After finishing and verifying each phase, stop and wait for explicit user review before starting the next phase — the user decides whether to commit, iterate further, or adjust the phase as-is before any further work begins. Never chain into the next phase on your own.

## Issue tracking

Work is tracked on GitHub Issues at `nian0601/Slush` (migrated from Trello). Three label groups, plus one standalone tag:

- **Priority**: `priority:p0` (drop-everything/blocking) through `priority:p3` (low/someday); `priority:p2` is the normal default.
- **Size**: `size:s`/`size:m`/`size:l`, a holistic judgment call made by `plan-issue` on the assembled phase breakdown — how long the plan would realistically take to implement, not a mechanical count. Rough anchors (sessions share no memory of past calls, so these keep the judgment calibrated): `size:s` ≈ under a couple hours, `size:m` ≈ half a day to a day, `size:l` ≈ multiple days/sessions.
- **Project**: `project:engine` or `project:actiongame` — which part of the codebase the issue belongs to. Just these two for now; revisit if it needs to be more granular (e.g. per-game, as BossMonster/TopDownGame become active) or dropped.
- **`no-plan`**: an additional tag on issues filed without a phase breakdown (quick/direct path, or a deliberately unplanned issue) — layered on top of whatever priority/size/project labels still apply.

Four project skills (`.claude/skills/`) drive the workflow:

- `/plan-issue` — exhaustively clarifies a task before it's filed. Never assumes or infers scope, priority, or project — always asks. Breaks the work into phases, each with enough self-contained detail (files/systems, approach, verification) that `/implement-issue` can act on it later without re-deriving context. Confirms the full draft with the user before creating anything.
- `/create-issue` — writes a drafted (or quick, unplanned) issue to GitHub via `gh issue create`, with the label taxonomy above and a per-phase checklist body.
- `/find-issue <time budget>` — lists open issues, shortlists by priority and a coarse size-vs-budget heuristic, and proposes the best fit for confirmation rather than starting work automatically.
- `/implement-issue <number-or-url>` — loads an issue's phase breakdown and works through it phase by phase autonomously by default: commits and moves on whenever a phase's own verification passes cleanly, stopping only if something unexpected comes up (a deliberate, scoped exception to the phased-work convention above — see the skill for the exact stop conditions). Runs an automatic code-review pass and an end-of-run summary before proposing to merge/close.

Closing an issue and merging its branch back into `main` follow the same standing-permission rule as `git commit`: never do either without the user's go-ahead in that turn. Checking off a phase happens automatically as part of `/implement-issue`'s autonomous run.
