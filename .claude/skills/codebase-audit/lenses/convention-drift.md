## Your lens
Divergence from the conventions written down in `CLAUDE.md` (repo root) and
`~/.claude/CLAUDE.md`.

Question at each site: does this code follow the convention the project has already
committed to in writing?

Counts as a violation:
- member variables not named `my` + PascalCase, or the `my` prefix used on a local
  variable inside a function body
- function parameters not named `a`/`an` + PascalCase
- types, functions, or methods not PascalCase
- non-Allman braces, or space indentation instead of tabs
- Framework-library types missing the `FW_` prefix, or Engine/game code not under
  `namespace Slush`
- `#ifndef` header guards instead of `#pragma once`
- `<cassert>` or bare `assert` instead of `FW_ASSERT`
- new STL usage in Framework where a custom container already exists
  (`FW_GrowingArray`, `FW_String`)
- a bare global utility function called from more than one `.cpp` that should be
  wrapped in a namespace
- an enum/helper pair conceptually owned by one component that has been pulled out into
  its own file instead of staying colocated with that component
- a caller reaching for `EntityPrefab::GetComponentData<T>()` where the owning Component
  could expose a getter instead
- repeated or looped `Foo::GetInstance()` calls that should be hoisted into a local
  reference — `AssetRegistry`, `ComponentRegistry`, `DependencyTracker`, `Engine`
- a mutable string accessor or `ImGui::InputText` on an `AssetReference<T>`, which
  CLAUDE.md says stays read-only (`ImGui::Text`)
- a `Data` struct containing an `FW_StaticArray` with no explicit constructor calling
  `.Fill(...)`, since the default constructor does not zero-initialize

Does NOT count:
- pre-existing `std::string`/STL in Engine or game code — CLAUDE.md says this already
  occurs and is tolerated. Only flag NEW STL introduced in Framework.
- vendored SFML / ImGui, which follow their own style
- generated files and build artifacts

Severity guidance for this lens: naming and formatting drift is `low` unless it is
pervasive in one file. Structural conventions — the AssetReference read-only rule, the
STL-in-Framework rule, the missing `.Fill(...)` — are `medium`.

## Triage — run these first, read only the hits

This lens is almost entirely greppable, and it is the cheapest of the four when run that
way. Every violation above has a mechanical signature. **Do not read files to look for
convention drift — grep for it, and read only to confirm a hit is real.** Set `S` first:

    S="Solution/Engine Solution/Framework Solution/ActionGame Solution/BossMonster Solution/TopDownGame"

Expected hit counts are given so you can tell a working grep from a broken one. If a
grep returns wildly more than stated, your pattern is wrong — fix it rather than reading
everything it returned.

1. Structural conventions — these are `medium` severity, do them first (~18 files total):

       rg -l '#ifndef' $S -g '!imgui' -g '*.h'                                    # ~1 file
       rg -n '#include <cassert>|[^_[:alnum:]]assert\s*\(' $S -g '!imgui'         # ~2 files
       rg -n '#include <(vector|string|map|unordered_map|set|algorithm|memory)>' Solution/Framework   # ~1 file
       rg -n -B3 -A3 'InputText' $S -g '!imgui'                                   # ~5 files
       rg -n 'GetComponentData<' $S -g '!imgui'                                   # ~9 files
       rg -n -A5 'FW_StaticArray' $S -g '!imgui'                                  # ~10 files

   For `InputText`, the violation is specifically one bound to an `AssetReference` —
   check the surrounding context lines, not the bare hit. For `FW_StaticArray`, the
   violation is a `Data` struct holding one with no constructor calling `.Fill(...)`;
   the `-A5` usually shows you enough to decide.

2. Naming drift, via PCRE2 negative lookahead (`-P` is required):

       rg -P -n '^\t(?:mutable\s+|static\s+)?(?!my)[A-Za-z_][\w:<>,\*& ]*\s+(?!my)[a-z]\w*\s*(?:=[^=]|;|\[)' $S -g '!imgui' -g '*.h'      # ~12 files
       rg -P -n '\((?:const\s+)?[A-Za-z_][\w:<>&\*]*\s+(?!a[A-Z]|an[A-Z]|aa)[a-z]\w*\s*[,\)]' $S -g '!imgui' -g '*.h'                    # ~17 files

   Both over-select — they catch free-function declarations, locals in inline bodies, and
   `Data` struct fields. Read the hits to separate real member/parameter declarations
   from noise. This is the only part of this lens that needs real reading; batch it
   5–10 files per message.

3. Formatting — currently clean, so these should return nothing. A non-empty result is
   the finding:

       rg -l '^    [^ *]' $S -g '!imgui' -g '*.cpp' -g '*.h'   # space indent, expect 0
       rg -l '\)\s*\{\s*$' $S -g '!imgui' -g '*.cpp'           # K&R brace, expect 0

4. Hoisting drift — files calling `GetInstance()` more than twice are the candidates:

       rg -c 'GetInstance\(\)' $S -g '!imgui' -g '*.cpp' | sort -t: -k2 -rn | head -15

   Read only the top few and check whether the calls sit in a loop or repeat within one
   function body. A single call per function is fine and is not a finding.

If your greps come back near-empty across the board, that is a real result — say so in
Coverage and write a low-count file. Do not go reading the tree to manufacture findings.
