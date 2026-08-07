## Your lens
How the code handles the absence of a thing it asked for.

Question at each site that can return nothing — asset lookup, component lookup, file
read, find-by-name, array search: is the empty/failed case handled, and is it handled
the way CLAUDE.md says it should be?

Counts as a violation:
- a pointer returned from a lookup that is dereferenced without a null check
- a fallback object, placeholder entity, or per-call-site log wrapped around
  `AssetReference<T>::Get()`. CLAUDE.md is explicit: resolution failure is logged
  centrally by `ResolveDependency()`, call sites are bare `if (T* p = ref.Get()) …`,
  with no per-call-site fallback and never a spawned placeholder empty entity
- a genuine invariant handled as a soft recoverable absence instead of a hard
  `FW_ASSERT` — CLAUDE.md's stated example is `Level::Restart()`'s player spawn
- the inverse: a recoverable absence that hard-asserts and takes the process down
- a file or IO result whose failure branch silently continues with garbage state
- a lookup whose failure is swallowed so the caller cannot distinguish "absent" from
  "present but empty"

Does NOT count:
- lookups whose failure is genuinely impossible because the value was constructed in
  the same scope
- `FW_ASSERT` on programmer-error preconditions (index bounds, null arguments)
- vendored SFML / ImGui internals

## Triage — run these first, read only the hits

Every finding for this lens sits at a call that can return nothing. Find those calls,
then read the ~3 lines after each to see whether the result is checked. Set `S` first:

    S="Solution/Engine Solution/Framework Solution/ActionGame Solution/BossMonster Solution/TopDownGame"

1. **Read the contract first**, in one batched message — these four define what correct
   looks like, and every judgement depends on them:

       Solution/Engine/Core/Assets/AssetReference.h
       Solution/Engine/Core/Assets/AssetRegistry.h
       Solution/Engine/Core/Assets/DataAsset.h
       Solution/Framework/FW_Assert.h

2. Lookup sites with two lines of trailing context, so the check (or its absence) is
   visible in the grep output itself and you often need no `Read` at all:

       rg -n -A2 '\.Get\(\)|->Get\(\)'                       $S -g '!imgui' -g '*.cpp'
       rg -n -A2 'Find\w*\(|GetComponent|GetAsset|GetPrefab' $S -g '!imgui' -g '*.cpp'

   ~15 and ~38 files respectively. The `-A2` is the point — a hit followed by `if (`,
   `FW_ASSERT`, or a null test is fine and needs no further reading. Only hits whose
   next line dereferences the result are candidates.

3. The two CLAUDE.md-specific anti-patterns, which are pure grep:

       rg -n -B2 -A6 '\.Get\(\)' $S -g '!imgui' -g '*.cpp' | rg -n 'else|LOG|Print|Warn|placeholder|fallback'
       rg -n 'FW_ASSERT' $S -g '!imgui' -g '*.cpp'

   The first finds per-call-site logging/fallback around `Get()`, which CLAUDE.md
   forbids. The second finds hard asserts — check each for the inverse violation, a
   recoverable absence that kills the process.

4. IO failure paths:

       rg -n -A3 'ReadEntireFile|LoadFromFile|OpenFile|fopen|ifstream' $S -g '!imgui'

Read files only where the grep context is genuinely ambiguous about whether the result
is checked. Batch those reads 5–10 per message. Most hits resolve from context alone —
that is what keeps this lens affordable.
