## Your lens
Units of code whose cost to understand exceeds what they do for you. Two separate
questions — answer both, keep the findings distinct.

**Q1. Is this function too big for what it accomplishes?**
**Q2. Is this class shallow — a wide public interface with little behind it?**

This lens is judgment-heavy, so it runs under hard constraints. Follow them exactly;
they are what keep the output checkable.

### Constraint 1 — triage mechanically before you judge

Do not browse for things that "look big". Run the two sweeps below, then read **only the
line ranges they surface**. Both are verified against this repo and their expected output
is stated — if yours differs a lot, your command is wrong, not the repo.

Set the shell to the repo root first.

**Sweep A — function bodies of 60+ lines.** Two variants, and you need both. Files
wrapped in `namespace Slush` indent their function bodies one tab; files without a
namespace (most of `ActionGame/`, `BossMonster/`, `TopDownGame/`) brace at column 0.
Never run the column-0 variant against a namespaced file — it matches the `namespace`
block itself and reports a bogus 400-line "function".

    # namespaced files — tab-indented bodies (expect ~15 hits)
    find Solution -name '*.cpp' -not -path '*/imgui/*' -print0 \
    | xargs -0 awk 'FNR==1{s=0} /^\t\{/{s=FNR} /^\t\}/{if(s&&FNR-s>=60)print FNR-s"\t"FILENAME":"s"-"FNR; s=0}' \
    | sort -rn

    # non-namespaced files only — column-0 bodies (expect ~9 hits)
    for f in $(find Solution -name '*.cpp' -not -path '*/imgui/*'); do
      if ! grep -q '^namespace' "$f"; then
        awk -v F="$f" 'FNR==1{t=0} /^\{/{t=FNR} /^\}/{if(t&&FNR-t>=60)print FNR-t"\t"F":"t"-"FNR; t=0}' "$f"
      fi
    done | sort -rn

Together these yield about **24 candidates** across the whole repo. That is your entire
Q1 search space — the sweep is exhaustive at the 60-line threshold, so anything not in it
is under threshold and only reportable via the sub-60 criteria below (nesting depth 4+,
4+ tracked mutable locals, three distinct responsibilities). Do not go hunting for those;
note them only if you happen to see them while reading a candidate.

For calibration: the longest function here is **129 lines** (`UIBuilder.cpp:364-493`) and
only four exceed 100. A 60-line body is genuinely large in this repo — do not import
thresholds from larger codebases.

**Sweep B — interface width, for Q2** (expect ~12 hits at the 10+ threshold):

    find Solution -name '*.h' -not -path '*/imgui/*' | while read f; do
      n=$(grep -cE '^[[:space:]]+[A-Za-z_].*\(.*\)[[:space:]]*(const)?[[:space:]]*(\{.*\}|;)[[:space:]]*$' "$f")
      [ "$n" -ge 10 ] && echo -e "$n\t$f"
    done | sort -rn

Read candidate ranges with batched `Read` calls using `offset`/`limit` to pull just the
range — 5–10 in a single message. Do not read whole files when a 90-line range is the
unit under audit.

### Constraint 2 — every finding carries its numbers

A finding with no measurement is not a finding. Required in `evidence:`:

- functions — body line count, maximum nesting depth, number of branch points, and the
  count of mutable locals live across the whole body
- classes — number of public member functions, and total non-trivial implementation
  lines behind them (excluding one-line getters/setters and forwarding calls)

Numbers get spot-checked during consolidation. If yours are wrong your whole file is
discarded, so count rather than estimate. The sweeps already give you exact body line
counts — use those, do not re-estimate them.

### Constraint 3 — a function finding must name the split

State the line range that would become the extracted unit and what it would be called:
"lines 112-156 are a self-contained projectile-spawn step, extractable as
`SpawnProjectileBurst`". If you cannot name the seam, the function is long but not
badly structured — **do not report it.** Length alone is not a defect.

### Constraint 4 — banned vocabulary

Do not use "complex", "hard to read", "unwieldy", "messy", "code smell", "clean", or
"well-structured" in any claim. Name the mechanism instead: the nesting, the count, the
number of responsibilities, the flag that has to be tracked across 80 lines.

### Constraint 5 — hard cap of 12 findings

Report the 12 worst, ranked. If you have more candidates, that means you must compare
and cut, which is the point. Say in Coverage how many candidates you triaged.

### Counts as a violation

Q1 — functions:
- 60+ line body **and** a nameable split seam (Constraint 3)
- shorter than 60 lines but with nesting depth 4+, or 4+ mutable locals tracked across
  the whole body, or handling three or more clearly distinct responsibilities
- a body whose early section and late section share no data

Q2 — classes:
- 8+ public member functions where the implementation behind them is mostly one-line
  forwarding, getters, or setters, so the class adds interface surface without adding
  behaviour
- a class whose public interface exposes its internal representation such that callers
  must know the internals to use it correctly
- two or more classes with near-identical public interfaces that could be one type

### Does NOT count

- Getter-heavy `Component` classes. CLAUDE.md explicitly prefers exposing a getter on
  the owning Component (e.g. `PhysicsComponent::GetCollisionFlag()`) over callers
  reaching for `GetComponentData<T>()`. That is the chosen design — flagging it is
  wrong, and this exemption will otherwise eat your entire finding budget.
- The Framework container and math types — `FW_String`, `FW_GrowingArray`, `FW_Hashmap`,
  `FW_Math`, `FW_Vector2/3`. Sweep B ranks these at the very top (99, 67, 47, 34, 31
  declarations) and every one is a false positive: a wide interface is the entire job of
  a container, and CLAUDE.md mandates these exist instead of STL. **Skip them without
  reading.** They are the four highest hits, so this exemption saves most of Sweep B's cost.
- `Data` structs and POD types. Wide, trivial, public is their job.
- Long flat `switch` or `if/else` chains that dispatch on an enum with one short arm
  each, and long straight-line registration/serialization/ImGui-layout bodies. These
  are long but have depth 1 and no seam — Constraint 3 already excludes them, but check
  explicitly before reporting anything in `AssetParser`, `UIBuilder`, the `Dockables`,
  or animation track code. Several of Sweep A's top hits live in exactly those files.
- Files that are long because they contain many small functions. This lens is about
  units, not files.
- Anything in vendored SFML / ImGui.
