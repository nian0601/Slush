# Slush codebase review — Codex / GPT

Produced by: Codex (GPT), with three delegated investigations and primary-agent verification.

Review date: 2026-10-04  
Source snapshot: `main`, commit `8821f27`

The main recommendation is to make **ownership, data contracts, and automated verification explicit**. Those changes would improve stability now and give AI agents a much safer foundation for extending the project.

Three subagents investigated Framework, assets, and gameplay, followed by cross-checking of the principal findings. The review mapped 251 source/header files—about 24,800 lines—and examined selected paths across all five projects. Vendored SFML/ImGui internals were excluded.

The analysis was read-only: no code or data was changed, and no builds, tests, or games were launched, because those operations can write files. This report was subsequently saved at the user's request. Findings come from source inspection; their frequency in live gameplay was not measured.

## Highest-priority findings

### 1. Custom foundation types have unreliable contracts

These types underpin almost every subsystem, so their faults have a large reach:

- `FW_String::Length()` returns character count minus one; `"abc"` reports 2. Input validation consequently skips the final character.
- String equality compares cached hashes rather than contents, while some mutations leave the hash stale.
- String self-assignment empties the string; growing-array self-assignment reads freed storage.
- `FW_RandInt(0, 2)` produces only 0 or 2, making the middle choice unreachable. Real callers select enemies, drops, and upgrades.

Evidence: [FW_String.h](../Solution/Framework/FW_String.h#L415), [FW_GrowingArray.h](../Solution/Framework/FW_GrowingArray.h#L115), [FW_Math.h](../Solution/Framework/FW_Math.h#L33).

**Recommendation:** establish and test conventional contracts for length, equality, copying, lifetime, and random ranges. Migrate carefully: existing callers sometimes compensate for today's unusual behavior. For example, `Reserve()` intentionally changes logical size, so changing it requires reviewing callers.

For a remake, revisit the breadth of the custom-foundation decision. Preserve custom implementations where they serve a clear engine or learning goal; give every retained primitive a precise, independently tested contract.

### 2. Entity and world ownership can cause invalid memory access during cleanup

`EntityManager` deletes handle proxies before deleting entities. Each entity owns a handle whose destructor accesses its proxy. Pending spawns are also omitted from manager cleanup, and assigning an empty handle dereferences null.

ActionGame's `LevelState` allocates a manager and physics world but deletes neither. **Fixing those leaks alone would expose the unsafe manager destructor.** These changes need to be handled together.

Evidence: [EntityManager.cpp](../Solution/Engine/EntitySystem/EntityManager.cpp#L14), [EntityHandle.h](../Solution/Engine/EntitySystem/EntityHandle.h#L47), [LevelState.cpp](../Solution/ActionGame/StateStack/LevelState.cpp#L20).

**Recommendation:** introduce one ownership contract for a world: entities, pending commands, physics, and handles. Define reset and destruction order explicitly. Entity destruction must occur while physics remains available.

For a remake, use generation-checked entity IDs with documented world lifetime, or independently managed handle control blocks. Creation and removal should pass through one world command queue.

### 3. Asset loading can overwrite content without successful validation

`DataAsset::Load()` parses and automatically saves older versions without checking a complete parse result. Malformed structures can log errors yet still produce a valid root handle. Required numeric fields can silently retain defaults.

Saving overwrites the destination directly and exposes no success result to the asset. Serialization reconstructs recognized fields, so migration can remove information that the current code does not understand.

Evidence: [DataAsset.cpp](../Solution/Engine/Core/Assets/DataAsset.cpp#L7), [AssetParser.cpp](../Solution/Engine/Core/Assets/AssetParser.cpp#L297), [FW_FileProcessor.cpp](../Solution/Framework/FW_FileProcessor.cpp#L12).

**Recommendation:** separate loading, validation, migration, and saving. Publish assets only after validation succeeds. Reject unsupported future versions, return structured errors, and replace files atomically after a successful write.

For a remake, migrations would be explicit transformations with previewable diffs. Ordinary loading and validation would be read-only operations.

### 4. Fixed-step physics retains state at the wrong scope

`TickLimited()` uses a function-static accumulator shared by all physics worlds. It clears collision manifolds once before the catch-up loop; each subsequent substep appends new manifolds and resolves the previously accumulated ones again.

This introduces cross-world timing interference and stale collision work during frames containing multiple substeps.

Evidence: [PhysicsWorld.cpp](../Solution/Engine/Physics/PhysicsWorld.cpp#L197).

**Recommendation:** make the accumulator world-owned, clear solver scratch data per substep, and define gameplay collision-event accumulation separately.

For a remake, expose a pure simulation operation such as `Step(fixedDelta)`. Put catch-up scheduling outside physics, with explicit limits and diagnostics.

## Structural improvements

### 5. Give asset editing explicit commit/discard semantics

The editor modifies registry-owned assets directly. "Close Without Saving" removes tabs but preserves the modified values and dirty state. "Save As" reloads the original disk file, so it loses current unsaved edits, retains source-path metadata, and skips dependency resolution.

Dependency tracking is also a startup snapshot: changing a reference does not replace the recorded dependency edges.

Evidence: [AssetEditorDockable.cpp](../Solution/Engine/Core/Dockables/AssetEditorDockable.cpp#L118), [AssetStorage.h](../Solution/Engine/Core/Assets/AssetStorage.h#L95), [AssetReference.h](../Solution/Engine/Core/Assets/AssetReference.h#L49).

**Recommendation:** introduce edit sessions containing a draft or snapshot. Commit validates and publishes changes; discard restores the original; cloning copies current document values. Track dirty state through mutations and rebuild affected dependency edges on publication.

This would also provide a dependable API for AI-driven content edits.

### 6. Separate document schemas, runtime behavior, and editor presentation

`Asset` combines persistence, dependency resolution, UI methods, and icon metadata. `Component::BaseData` combines parsing, defaults, enablement, dependencies, and editor controls.

Adding a field therefore requires discovering its contract across several methods and layers. Independent validators and tests inherit unnecessary engine/editor dependencies.

Evidence: [Asset.h](../Solution/Engine/Core/Assets/Asset.h#L12), [Component.h](../Solution/Engine/EntitySystem/Component.h#L32).

**Recommendation:** extract plain document data and pure parsing/validation first, then move editor controls into adapters.

For a remake, one authoritative schema would declare field names, types, defaults, bounds, references, and versions. Serialization, routine editor controls, and reference documentation could derive from it, with custom adapters for complex cases.

### 7. Make game composition and execution order inspectable

ActionGame and TopDownGame each implement the same engine-owned `EntityManager::RegisterComponents()` member. Composition depends on which implementation is linked.

Component IDs arise from first use; registration order determines creation/update order; storage has a fixed 32-component limit. Requirements and scheduling dependencies remain implicit. For example, NPC movement consumes targeting during `PrePhysicsUpdate()`, while targeting refreshes during the later `Update()` phase.

Evidence: [ActionGame registration](../Solution/ActionGame/EntitySystem/EntityManager.cpp#L27), [TopDownGame registration](../Solution/TopDownGame/EntitySystem/EntityManager.cpp#L15), [Entity.cpp](../Solution/Engine/EntitySystem/Entity.cpp#L60).

**Recommendation:** give each game an explicit composition root that receives a registry. Validate duplicate registrations, capacities, required components, and prefab roles before gameplay. Declare update phases and intentional previous-frame dependencies.

The current numeric IDs were not observed as persisted IDs; this finding concerns extension and startup fragility.

### 8. Scope services, clocks, and randomness to each simulation

TopDownGame components obtain navigation and rewards through a global "current level." ActionGame also uses a global manager in some component operations despite entities already knowing their owning manager.

Timers use process time, including direct wall-clock weapon cooldowns. Pausing gameplay updates does not pause that clock. Random helpers share process-wide state.

Evidence: [MovementComponent.cpp](../Solution/TopDownGame/Components/MovementComponent.cpp#L28), [Level.cpp](../Solution/TopDownGame/Level/Level.cpp#L133), [ProjectileShootingComponent.cpp](../Solution/ActionGame/Components/ProjectileShootingComponent.cpp#L55).

**Recommendation:** provide world-owned services for navigation, resources, spawning, simulation time, and seeded randomness. Distinguish simulation time from UI/real time.

This enables gameplay, editor previews, and test worlds to coexist. Fixed ticks and controlled RNG would improve reproducibility, although complete determinism also requires consistent execution order and numerical behavior.

### 9. Treat tower placement and navigation changes as one transaction

Placement validates the footprint, spends resources, spawns a tower, and cuts the navmesh. Existing enemies calculate paths on entering the world and never invalidate them after mesh changes. New enemies with no path are removed.

Evidence: [Level.cpp](../Solution/TopDownGame/Level/Level.cpp#L91), [MovementComponent.cpp](../Solution/TopDownGame/Components/MovementComponent.cpp#L28).

**Recommendation:** define whether blocking routes is legal. Then implement preview → validate → commit, including the chosen connectivity policy and path invalidation. Give mutable navigation a revision that cached paths can check.

This is a consistency gap; the intended route-blocking behavior remains a product decision.

### 10. Separate input sampling and gameplay lifetime from editor state

The engine skips keyboard/mouse state updates when ImGui captures input. Existing pressed/down/released states therefore remain unchanged while gameplay continues reading them. A held movement key can remain logically down after capture begins.

Gameplay also lives inside editor layouts. Window layout APIs have different ownership rules: directly assigned layouts are omitted from destruction, and `SetAppLayout()` bypasses the guarded transition flow.

Evidence: [Engine.cpp](../Solution/Engine/Core/Engine.cpp#L111), [PlayerControllerComponent.cpp](../Solution/ActionGame/Components/PlayerControllerComponent.cpp#L28), [Window.cpp](../Solution/Engine/Graphics/Window.cpp#L282).

**Recommendation:** always advance raw input state, then route an appropriate input snapshot to gameplay. Give gameplay sessions independent ownership and unify layout ownership/transition handling.

### 11. Create an independent verification path with useful failure output

Framework's test runner currently runs one file-processor round-trip test. TopDownGame adds useful navigation and wave-scaling tests, but tests execute during game startup.

`-hidewindow` still constructs the SFML window, renderer, and ImGui stack. Tower scenarios require key sequences and report failures through logs. Assertions accept explanatory messages but discard them before breaking/crashing.

Evidence: [FW_UnitTestSuite.cpp](../Solution/Framework/FW_UnitTestSuite.cpp#L100), [Engine.cpp](../Solution/Engine/Core/Engine.cpp#L65), [TowerTestState.cpp](../Solution/TopDownGame/StateStack/TowerTestState.cpp#L64), [FW_Assert.h](../Solution/Framework/FW_Assert.h#L11).

**Recommendation:** provide standalone foundation tests, read-only asset validation, and simulation scenarios that terminate with meaningful exit codes. Include expression, message, file, and line in assertion failures.

Start with the concrete defects above and important behavioral boundaries: world reset, malformed migration, multiple physics substeps, placement, damage, and wave transitions.

### 12. Make build and agent setup reproducible from the checkout

Build configuration is duplicated across projects, includes depend on `SolutionDir`, and Debug/Release executables share destinations under `Workbed`. Runtime assets, tracked editor settings, and generated files occupy the same workspace.

The instruction entry point also requires a personal absolute-path guidance file, which a fresh machine or remote agent may lack.

Evidence: [TopDownGame.vcxproj](../Solution/TopDownGame/TopDownGame.vcxproj#L73), [Engine.vcxproj](../Solution/Engine/Engine.vcxproj#L73), [AGENTS.md](../AGENTS.md#L3).

**Recommendation:** centralize supported build configurations and expose repository-owned build/check commands. Separate outputs by configuration and keep generated runtime state outside authored content. Put essential project policies in the repository; retain personal guidance as an optional overlay.

Windows-only development can remain a deliberate constraint while these improvements are made.

## What to design differently for an AI-heavy remake

Organize the project around independently exercisable contracts:

| Boundary | Responsibility | Benefit for AI work |
|---|---|---|
| Foundation | Value types, ownership, math, diagnostics | Fast checks of shared invariants |
| Content documents | Schemas, parsing, validation, migrations | Safe content changes with precise errors |
| Simulation | World state, commands, events, clock, RNG | Repeatable scenarios without graphics |
| Game modules | Explicit composition and game rules | Smaller, discoverable change scope |
| Platform/presentation | SFML, rendering, input devices | Backend changes with controlled impact |
| Editor | Drafts, commands, undo, publication | Same operations available to humans and automation |

The most valuable workflow changes would be:

- **One authoritative definition per contract.** Avoid duplicating schema knowledge across parsing, defaults, UI, and registration.
- **Commands usable by both editor and automation.** Placement, spawning, reference changes, and migration should return structured outcomes.
- **Reproduction artifacts.** Record scenario, content revision, seed, tick sequence, and commands when a test fails.
- **A short local verification loop.** An agent should validate its affected module without opening a game or modifying authored assets.
- **Explicit dependency boundaries.** Narrow public APIs and self-contained headers make parallel work easier to reason about.
- **Repository-contained guidance.** Document module responsibilities, invariants, and exact verification commands alongside the code.

Preserve the existing Engine/Framework/game separation, typed asset references, two-pass dependency resolution, version metadata, and pure `WaveScaling` functions. Those are useful starting points.

For shared gameplay, extract narrow contracts such as health changes and death events. ActionGame's animation/UI responses and TopDownGame's rewards can subscribe through game-specific policies. Their entire level implementations have substantially different responsibilities and should retain that distinction.

## Recommended order

1. Fix foundation correctness and world teardown together with focused regression checks.
2. Make asset loading non-destructive and saving transactional.
3. Correct physics substep state and input-state advancement.
4. Extract independent tests, asset validation, and controllable simulation time/RNG.
5. Introduce explicit game composition, document schemas, and editor edit sessions.
6. Tighten module/build boundaries as those seams become established.

## Coverage limits

The review was focused rather than exhaustive. It did not fully audit numerical algorithms, rendering/UI behavior, every asset migration, or the complete content corpus. Those areas remain coverage gaps rather than cleared areas.
