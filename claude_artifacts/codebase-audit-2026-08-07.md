# Slush codebase audit — 2026-08-07

4 lenses over ~20k lines of source. 38 findings after consolidation (38 raw, 1 dropped,
1 added during consolidation).

**This audit is incomplete.** All four agents were killed mid-sweep by a session usage
limit. Every file below is a checkpoint, not a finished sweep. See Coverage.

## Summary
| lens | high | medium | low | total |
|---|---|---|---|---|
| absence-handling | 7 | 8 | 3 | 18 |
| ownership-lifetime | 1 | 2 | 4 | 7 |
| depth-and-size | 2 | 4 | 2 | 8 |
| convention-drift | 0 | 0 | 4 | 4 |
| added in consolidation | 0 | 1 | 0 | 1 |
| **total** | **10** | **15** | **13** | **38** |

Cross-lens agreement: **0 findings flagged independently by 2+ lenses.** The lenses did
not overlap at all — closest approach was `EntityManager.cpp:50` (absence) vs `:14`
(ownership), 36 lines apart and unrelated defects. This means no finding here has
independent corroboration; every one rests on a single agent, which is why the
verification below matters more than usual.

## Coverage — read this before treating any area as clean

| lens | scope covered | status |
|---|---|---|
| absence-handling | ~90% of .cpp; most headers unread | most complete |
| ownership-lifetime | Engine ~95%, ActionGame ~40%, BossMonster/TopDownGame **0%** | partial |
| depth-and-size | both mechanical sweeps **complete**; 20 of 66 candidates read | triage complete, reading partial |
| convention-drift | Framework 100%, Engine ~20%, ActionGame ~8%, others **0%** | **~35% — least complete** |

Specific gaps that likely hide findings:

- **`Solution/BossMonster/` (11 files) and `Solution/TopDownGame/` (5 files) were read by
  only one lens.** Treat both as unaudited.
- The uninitialized-raw-pointer defect behind five absence findings lives in *headers*,
  and most ActionGame component headers went unread. That pattern is likely
  under-reported.
- `convention-drift` did run whole-scope targeted greps for its structural rules
  (`#ifndef`, asserts, `InputText`, `FW_StaticArray`, `GetInstance`), so its **medium**
  band is more trustworthy than its file coverage suggests. Its naming coverage is not.
- `depth-and-size` finished both sweeps, so its candidate list is exhaustive at the
  threshold; 46 candidates in the 50-72 line tail were never opened.

## Verification performed during consolidation

**Spot-checked `depth-and-size`'s numbers against an independent brace sweep — all
correct.** UIBuilder 364-493 (129), PhysicsWorld 134-195 (61), Animation 291-381 (90) and
210-288 (78), PhysicsShapes 289-368 (79), AssetEditorDockable 159-269 (110) all match
exactly. `BaseSprite.h` public function count independently recounted at 25, matching. One
1-line offset on PhysicsShapes CircleVsPolygon (agent 17-107, sweep 15-108) — immaterial.
That file's measurements are trustworthy.

**Independently confirmed by reading the source:** `LevelState` ctor `new`s both objects
and its dtor deletes neither; `PhysicsComponent.h:44` declares `PhysicsObject* myObject;`
with no initializer; `DataAsset::Load` has no `rootHandle.IsValid()` check before
`OnParse`/`Save`; `FW_ASSERT` has no NDEBUG guard and is live in every configuration.

**One agent claim refuted.** `absence-handling` asserted that `FW_ARRAY_BOUNDS_CHECK`
"is not defined anywhere in the solution". It is — `FW_Includes.h:3`, and
`Solution/Engine/stdafx.h` includes `FW_Includes.h` first, so bounds asserts are active in
Engine translation units. The agent noticed this itself in its final message before being
killed but never corrected the file. Consequences applied below.

## High severity

### 1. Parse failure resaves the asset with defaults, destroying the file
`Solution/Engine/Core/Assets/DataAsset.cpp:12` — *absence-handling* — CONFIRMED
(independently verified)

`AssetParser::Load()` returns an invalid `Handle` on read failure. `DataAsset::Load()`
never checks it: it calls `OnParse(rootHandle, 0)`, and `NeedsUpgrade(0)` is true for
every asset (all `DEFINE_ASSET` versions are >= 1), so `Save()` writes the
default-constructed in-memory object back over the file.

**Failure:** an asset file that is locked, zero-length, or briefly unreadable during
`AssetRegistry::LoadAllAssets()` is silently overwritten on disk with defaults. The log
shows the parse errors, but the data is already gone. This is the only finding here that
destroys user data, which is why it leads.

### 2. `LevelState` leaks an EntityManager and a PhysicsWorld per playthrough
`Solution/ActionGame/StateStack/LevelState.cpp:33` — *ownership-lifetime* — CONFIRMED
(independently verified)

Ctor lines 20-21 `new` both; the dtor only does `FW_SAFE_DELETE(myLevel)` and
`SetEntityManager(nullptr)`. No `delete` of an `EntityManager` or `PhysicsWorld` exists
anywhere in the repo.

**Failure:** Start Game → pick character → ESC → Main Menu leaks one of each, plus the
PhysicsWorld's `myCircleObject`/`myCircleShape`, every loop.

**Must be fixed together with finding #12** — adding the missing delete converts a latent
destructor-ordering bug into a live use-after-free.

### 3. `PhysicsComponent::myObject` is left uninitialized on shape-creation failure
`Solution/Engine/EntitySystem/Components/PhysicsComponent.h:44` — *absence-handling* —
CONFIRMED (independently verified)

`myObject` has no `= nullptr`, unlike every other raw pointer member in the codebase. When
`matchSprite` is set and the sprite's size is 0, the ctor logs and returns without
assigning it — but the component is still constructed and added to the entity.

**Failure:** `Update()` dereferences indeterminate memory; `~PhysicsComponent()` calls
`delete` on it.

### 4. `Level` dereferences a `LevelData` pointer it never null-checks
`Solution/ActionGame/Level/Level.cpp:31` — *absence-handling* — CONFIRMED

`GetAsset<LevelData>("level_main")` returns nullptr + warning on a miss; `Restart():74`
and `HandleEnemyWaves():154-157` dereference it. `Restart()` is called unconditionally
from `LevelState`'s ctor.

*Amended during consolidation:* the agent's secondary claim — that an empty `myEnemyWaves`
would read wild memory — is **wrong**, since bounds asserts are active. It would assert.
The primary null-deref claim stands. Per CLAUDE.md this is arguably the `Level::Restart()`
invariant case that should hard-assert rather than null-deref.

### 5. `WeaponComponent` binds a reference to an unresolved `AssetReference::Get()`
`Solution/ActionGame/Components/WeaponComponent.cpp:312` — *absence-handling* — CONFIRMED

`WeaponData* startingWeapon = ...myWeaponData.Get(); myWeapons.Add(new Weapon(myEntity,
*startingWeapon));` — no guard, and `Weapon`'s ctor immediately does
`&myWeaponData.myRanks[0]`. CLAUDE.md mandates the bare `if (T* p = ref.Get())` form,
which this file uses correctly 50 lines earlier at :263.

### 6. `CharacterAnimationComponent` dereferences an unresolved animation-set reference
`Solution/ActionGame/Components/CharacterAnimationComponent.cpp:53` — *absence-handling* —
CONFIRMED

`myAnimationSet = data.myAnimationSetID.Get();` (member has no initializer), dereferenced
unguarded at :53 and :69. A prefab with `characteranimation` but no `animationSetID` field
crashes on the first `PrePhysicsUpdate()` reaching `PlayMovementAnimation()`.

### 7. `AnimationComponent` dereferences its `SpriteComponent` lookup unchecked
`Solution/Engine/EntitySystem/Components/AnimationComponent.cpp:27` — *absence-handling* —
CONFIRMED

Four sites (`:27`, `:64`, `:107`, `:146/148`) chain
`GetComponent<SpriteComponent>()->GetSprite()` with no check, while the sibling
`SpriteComponent::Update():233` guards the mirror-image lookup and logs. A prefab enabling
`animation` without `sprite` crashes on the first update.

### 8. `Tilemap` dereferences two texture lookups on the boot path
`Solution/ActionGame/Level/Tilemap.cpp:14` — *absence-handling* — CONFIRMED

Both `GetAsset<Texture>("RA_Ground_Tiles")` and `"RA_Crypt"` are unchecked, then
dereferenced in `SetupGroundSprites()`/`AddSprite()`. Constructed from `Level`,
`MainMenuState` and `CharacterSelectionState`, so a missing texture crashes before the
main menu renders.

### 9. `UIBuilder::CalculateSizeAlongAxis` — 129 lines, depth 5, 10 mutable locals
`Solution/Engine/UI/UIBuilder.cpp:363` — *depth-and-size* — CONFIRMED (numbers verified)

The longest function in the repo. Holds a measurement pass, a grow-space distribution
solver, an off-axis clamp pass and a recursive descent in one body; `innerContentSize` is
written in four separate passes across all 129 lines. Confirmed not to be an excluded flat
ImGui-layout body — it contains no ImGui calls.

**Split:** 378-403 → `MeasureChildrenAlongAxis`; 417-464 → `DistributeGrowSpace`. Leaves a
~30-line driver.

### 10. `PhysicsWorld::Tick` mixes broadphase and solver at depth 5
`Solution/Engine/Physics/PhysicsWorld.cpp:133` — *depth-and-size* — CONFIRMED (numbers
verified)

61 lines, two responsibilities with a clean seam at line 170: 137-169 write `myManifolds`,
171-194 read it, and no local state crosses.

**Split:** `DetectCollisions()` + `SolveAndIntegrate()`; `Tick()` becomes two calls.

## Medium severity

### 11. `CreateEntity` spawns a placeholder empty entity on prefab miss
`Solution/Engine/EntitySystem/EntityManager.cpp:50` — *absence-handling* — CONFIRMED

Does precisely what CLAUDE.md's Code style section forbids: per-call-site logging *and* a
spawned placeholder empty entity. Callers dereference the result as if real. Deleting the
`Wall` prefab spawns one invisible, non-colliding, never-removed entity per wall tile.

### 12. `~EntityManager` destroys proxies before the entities that reference them
`Solution/Engine/EntitySystem/EntityManager.cpp:14` — *ownership-lifetime* — CONFIRMED

`myProxyStorage.DeleteAll(); myEntities.DeleteAll();` in that order; each `~Entity()` runs
`~EntityHandle()` which writes `myProxy->myRefCount--` into freed memory.
`DeleteAllEntities():59` gets the ordering right — the destructor does not.

**Latent only because finding #2 means `~EntityManager()` never runs. Fix both together,
proxies last.**

### 13. Include-order determines whether array bounds checks exist at all
`Solution/Framework/FW_GrowingArray.h:13` — *added during consolidation* — CONFIRMED

`FW_ARRAY_BOUNDS_CHECK` is defined in `FW_Includes.h:3`, immediately before it includes
`FW_GrowingArray.h`. But `FW_GrowingArray.h` is `#pragma once` and never defines the macro
itself, so any translation unit that reaches `FW_GrowingArray.h` before `FW_Includes.h`
compiles it with every bounds assert stripped — and `#pragma once` prevents the later
`FW_Includes.h` from restoring them. 15 headers include `FW_GrowingArray.h` directly.

Engine TUs are safe today because `stdafx.h` includes `FW_Includes.h` first. The safety is
incidental, it is invisible at the point of use, and it silently produced a wrong
conclusion in this very audit. Define the macro inside `FW_GrowingArray.h`, or have that
header include the definition itself.

### 14. `AssetRegistry::GetAssetStorage`'s assert is inert and returns the wrong storage
`Solution/Engine/Core/Assets/AssetStorage.cpp:47` — *absence-handling* — CONFIRMED

`FW_ASSERT("Failed to find storage for assettype");` passes a string literal as the
*condition*. A non-empty literal is always truthy, so the assert body is dead code and the
function falls through to `return *myAssetStorages[0];` — a different asset type. Every
other call in the file uses the two-argument form.

**Failure:** a stale dependency key yields a misleading `Textures: '<name>' not found.`
warning, and the caller cannot distinguish "unknown type" from "asset absent".

### 15. `Text`'s constructor dereferences `Font::GetSFMLFont()` unchecked
`Solution/Engine/Graphics/Text.cpp:17` — *absence-handling* — CONFIRMED

`Font::Load()` deliberately handles failure by logging and nulling `mySFMLFont`;
`Text::Text` then does `new sf::Text(*aFont.GetSFMLFont())`, converting a handled,
recoverable IO failure into a crash on the app-init path for both ActionGame and
BossMonster.

### 16. `Animation` hard-asserts on a missing spritesheet texture
`Solution/Engine/Graphics/Animation/Animation.cpp:64` — *absence-handling* — CONFIRMED
(FW_ASSERT semantics independently verified)

`FW_ASSERT(myTexture != nullptr, ...)` during `LoadAllAssets()`. `FW_ASSERT` has no NDEBUG
guard — its body is `__debugbreak()` then a deliberate null write — so it takes the
process down in *every* configuration. CLAUDE.md reserves hard asserts for genuine
invariants; every other texture reference in the codebase treats absence as recoverable.

**Failure:** renaming any texture referenced by an `.anim` file crashes all three games
during startup, with no log line naming the animation.

### 17. Animation-by-name lookups cached in uninitialized pointers, dereferenced later
`Solution/ActionGame/Components/HealthComponent.cpp:39` — *absence-handling* — CONFIRMED

`myDamageAnimation`, `myDashAnimation`, `myBlinkAnimation`, `mySpriteSheetAnimation`,
`myUpgradeData` across `HealthComponent`, `PlayerControllerComponent`, `WeaponComponent`
and `StatsComponent` — assigned from `GetAsset<T>(...)` with no check, most declared
without initializers. Deleting `Blink.anim` logs a warning at spawn, then crashes when the
entity first takes damage.

### 18. `Level::Update` dereferences two component lookups unguarded
`Solution/ActionGame/Level/Level.cpp:48` — *absence-handling* — CONFIRMED

Guards the player entity, then fetches `ExperienceComponent` and `WeaponComponent` and
dereferences both without checks — while `IsPlayerDead():98` in the same file uses the
guarded form. Same shape in `UpgradeStatsState.cpp:34-35`, `UpgradeWeaponState.cpp:35`,
`PauseState.cpp:102/140`.

### 19. Editor dereferences `GetSFMLTexture()` on textures that failed to load
`Solution/Engine/Core/Dockables/TextureViewerDockable.cpp:37` — *absence-handling* —
CONFIRMED

`Texture::Load()` signals failure by nulling `mySFMLTexture`, but the asset stays
registered. `TextureViewerDockable` iterates *all* registered textures and passes
`*texture->GetSFMLTexture()` to `ImGui::Image`. Note the guarded call sites
(`SpriteComponent.cpp:145`, `CharacterInfo.cpp:49`, …) are also exposed: their
`if (const Texture* t = ref.Get())` proves the asset exists, not that its bitmap loaded.

### 20. `Weapon::ShootProjectile` chains two component lookups on a fresh entity
`Solution/ActionGame/Components/WeaponComponent.cpp:268` — *absence-handling* — CONFIRMED

`projectile->GetComponent<PhysicsComponent>()->myObject->...` and the `SpriteComponent`
equivalent, unguarded — while the *next* statement uses the guarded form for
`DamageDealerComponent`. Compounds with finding #3: even when the PhysicsComponent exists,
`->myObject` may be the uninitialized pointer. Identical pair at
`ProjectileShootingComponent.cpp:70-71`.

### 21. `Window` never deletes the `IAppLayout` it owns
`Solution/Engine/Graphics/Window.cpp:47` — *ownership-lifetime* — CONFIRMED

`SetAppLayout()` establishes ownership by deleting the previous layout, but `~Window()`
deletes only `myRenderer` and `myRenderWindow`.

**Failure:** on any normal exit, `~GameLayout()` never runs, so `myStateStack->Clear()`
never runs, so every live state — including a live `LevelState`, its `Level`, and all
dockables — leaks. Transitively `Level::~Level()`'s `DeleteAllEntities()` never runs at
shutdown either.

### 22-25. Four oversized physics/animation/editor functions
*depth-and-size* — all CONFIRMED, numbers verified

- `PhysicsShapes.cpp:16` `CircleVsPolygon` — 90 lines, 6 locals. Split: 26-40 →
  `FindLeastPenetratingFace`; 58-101 → `ResolveNearestFeatureContact`. The same file
  already factors equivalent steps out for `PolygonShape::TestCollision`.
- `PhysicsShapes.cpp:288` `PolygonShape::TestCollision` — 79 lines, **9** mutable locals,
  and lines 347-353 / 355-361 are the same four statements differing only by array index.
  Split: 345-364 → `CollectClippedContacts`, duplicated pair collapsed to a 2-iteration loop.
- `Animation.cpp:290` `HandleTextureInteraction` — 90 lines, depth 5. Split: 313-324 →
  `RecomputeFrameGrid`; 330-339 → `GetRowHighlightRange`.
- `Animation.cpp:209` `HandleSpritesheetImport` — 78 lines, depth 5; the only non-UI logic
  (237-258) is buried inside a popup-modal widget tree. Split: → `ImportSpritesheetFrames()`.

## Low severity

- `Solution/Engine/EntitySystem/EntityPrefab.h:71` — `GetComponentData<T>()` dereferences a
  possibly-null base-data pointer; latent since all current components are registered. *(absence)*
- `Solution/Framework/FW_FileSystem.cpp:248` — `ReadEntireFile()` discards `fopen_s`'s
  result then `fseek`s the handle; currently dead code behind `USE_BINARY_FILE_PROCESSING == 0`. *(absence)*
- `Solution/Framework/FW_XMLParser.cpp:96` — `GetStringAttribute` returns nullptr while
  `GetIntAttribute` returns 0; inconsistent absence signalling, no in-tree callers. *(absence)*
- `Solution/Engine/StateStack/StateStack.h:23` — no destructor, so states leak unless
  `Clear()` is called first; contract undocumented and unenforced. *(ownership)*
- `Solution/Engine/EntitySystem/Components/SpriteComponent.cpp:242` — caches an
  `AnimationRuntime*` that `AnimationComponent` deletes; dangles but is only null-tested,
  so the visible symptom is an animation that can never replay. *(ownership)*
- `Solution/Engine/Core/Dockables/AssetEditorDockable.cpp:411` — copies an owning-pointer
  array by value (missing `&`); safe only incidentally. Same at `ContentBrowserDockable.cpp:54`,
  `TextureViewerDockable.cpp:23`. *(ownership)*
- `Solution/Engine/Core/Dockables/IAppLayout.cpp:79` — `myCloseRequestBlocker` can outlive
  the `Dockable` it points to. Verified unreachable today: no closable Dockable overrides
  `HasUnsavedChanges()`. Live the moment one does. *(ownership)*
- `Solution/Engine/Graphics/BaseSprite.h:11` — 25 public functions over ~28 lines of real
  logic; 5 shadow members duplicate `sf::Shape` state and 5 getters have zero callers. *(depth)*
- `Solution/Engine/Core/Dockables/AssetEditorDockable.cpp:158` — `OnBuildUI` 110 lines,
  depth 6. Split: 240-267 → `HandleAssetDragDrop()`; 192-204 → `BuildTabLabel()`. *(depth)*
- `Solution/Framework/FW_Intersection.h:6` — `sign()` is not PascalCase; sole outlier in
  the file. *(convention)*
- `Solution/Framework/FW_Matrix22.h:26` — params `a00`/`a01`/`a10`/`a11` are `a`-prefixed
  but not PascalCase. *(convention)*
- `Solution/Framework/FW_Murmur.h:5` — `#ifndef` guard instead of `#pragma once`; verbatim
  public-domain import, so arguably intentional. *(convention)*
- `Solution/Engine/Core/Dockables/DependencyTrackerDockable.cpp:146` — `AssetRegistry::GetInstance()`
  re-fetched per loop iteration via `ResolveAssetFromKey`. *(convention)*
- `FW_GrowingArray.h:2`, `FW_String.h:3`, `FW_StaticArray.h:2` — include `assert.h`/`<cassert>`
  against CLAUDE.md; no bare `assert(` calls remain, so these are dead includes.
  *(missed by convention-drift; found in consolidation)*

## Dropped during consolidation

- **`Level.cpp:141` "EnemyPrefabs pick indexes an array that may be empty"** (absence,
  SUSPECTED). Dropped on two independent grounds: `FW_ARRAY_BOUNDS_CHECK` *is* defined
  (`FW_Includes.h:3`) and reaches Engine TUs via `stdafx.h`, so an out-of-range index
  asserts rather than reading wild memory; and every wave in
  `Workbed/ActionGame/Data/Levels/level_main.ldata` has a populated `enemyPrefabs`. The
  underlying `FW_RandInt(0, -1)` → `rand() & 0xFFFFFFFF` behaviour is real but unreachable
  from shipped data.

## Proposed follow-up

Suggested as five issues on `nian0601/Slush`, batched so each is one coherent work item
rather than one issue per finding.

1. **"Asset load failures corrupt data and crash startup"** — `project:engine`, highest
   priority. Findings #1, #14, #15, #16, #19. All are the asset/IO layer failing to
   distinguish absent from broken. #1 destroys files and should be fixed first.

2. **"Fix EntityManager/PhysicsWorld/IAppLayout ownership at teardown"** — `project:engine`.
   Findings #2, #12, #21, plus the `StateStack` and `AssetEditorDockable` copy items.
   **#2 and #12 must land in the same change** — fixing the leak without reordering the
   destructor creates a use-after-free.

3. **"Guard component and asset lookups in ActionGame"** — `project:actiongame`. Findings
   #3, #5, #6, #7, #17, #18, #20, plus `EntityPrefab.h:71`. Largest batch; consider
   splitting engine-side (#3, #7) from game-side. #3's missing `= nullptr` is a one-line
   fix worth doing immediately and separately.

4. **"Remove the placeholder-entity fallback from EntityManager::CreateEntity"** —
   `project:engine`. Finding #11 alone; it is an explicit CLAUDE.md violation and the fix
   changes failure behaviour for callers, so it deserves its own issue.

5. **"Split oversized physics, UI-layout and animation functions"** — `project:engine`,
   lowest priority, pure structure. Findings #9, #10, #22-25 and the `AssetEditorDockable`
   / `BaseSprite` items. Every one carries a named seam and verified measurements.

Convention-drift's four low findings are not worth an issue on their own — fold them into
whichever change next touches those files, except the `assert.h` includes, which fit
naturally into batch 2.

**Before acting on "no findings here" for any area:** re-run the audit for
`BossMonster/`, `TopDownGame/`, and convention-drift over Engine/ActionGame. Those were
never swept.
