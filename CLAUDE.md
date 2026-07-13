# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

Slush is a personal, hand-rolled C++ game engine (namespace `Slush`) built on vendored **SFML** (`SFML/`) for windowing/graphics/audio and vendored **Dear ImGui** (`Solution/Engine/imgui/`) for editor tooling. It's a single Visual Studio solution (`Solution/Solution.sln`) containing:

- `Solution/Engine/` — shared engine code (windowing, input, rendering, asset system, ImGui-based editor dockables)
- `Solution/Framework/` — low-level, STL-avoiding utility library (containers, math, string/hashing, file I/O, assertions, unit tests) — all types prefixed `FW_`
- `Solution/ActionGame/`, `Solution/BossMonster/`, `Solution/TopDownGame/` — separate game executables built on Engine + Framework

`TopDownGame` currently hosts the in-progress navmesh generation/cutting work (`Navmesh.h/.cpp`, `FW_Intersection.h`).

## Build

Windows-only. Open `Solution/Solution.sln` in Visual Studio and build/run from the IDE (Debug|x64 or Release|x64). There is no CLI/CMake build path — don't invent an `msbuild` or `cmake` command.

## Code style

These conventions differ from typical C++ defaults — follow them in this codebase:

- Member variables: `my` + PascalCase (`myVertices`, `myHasAnchor`)
- Function parameters: `a`/`an` + PascalCase (`aVertex`, `anExpression`)
- Types/functions/methods: PascalCase (`GetVertex`, `CreateEdge`)
- Framework-library types are prefixed `FW_` (`FW_Vector2`, `FW_GrowingArray`, `FW_Intersection`) instead of being namespaced under a nested namespace — some are `namespace FW_Xxx { }` blocks, others are `FW_Xxx`-prefixed template classes
- Engine/game code uses a real namespace: `Slush::`
- Braces: Allman style (opening brace on its own line)
- Indentation: tabs, not spaces
- `#pragma once` for header guards, never `#ifndef`
- Framework code prefers custom containers (`FW_GrowingArray`, `FW_String`) over STL equivalents; `std::string`/STL do show up occasionally in Engine/game code
- Assertions use the `FW_ASSERT` macro (`Solution/Framework/FW_Assert.h`), not `<cassert>`

## Testing

There's no third-party test framework (no GoogleTest/Catch2). `Solution/Framework/FW_UnitTestSuite.h/.cpp` defines individual `TestXxx()` functions called from `FW_UnitTestSuite::RunTests()`, which each game's `main.cpp` invokes automatically at startup. Assertions inside tests use `FW_ASSERT`.

Note: commit messages mentioning "test" for the navmesh/intersection work (e.g. testing `FW_Intersection` against navmesh triangles) refer to **interactive/visual runtime checks** in `Navmesh.cpp`'s `Render()`/`Update()` (drag the mouse, see highlighted triangles/intersection points) — not entries in `FW_UnitTestSuite`. Don't conflate the two when asked to "add a test."

## Current WIP state

Navmesh cutting (`Navmesh.cpp`) is under active development and known incomplete: cutting arbitrary convex shapes mostly works, but resulting vertices aren't guaranteed to land exactly at cut positions.

## Repo etiquette

Solo project — commit directly to `main`, no branch/PR convention to follow.
