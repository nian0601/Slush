## Your lens
Ownership and lifetime of heap allocations.

Question at each site: for this allocation, is there exactly one owner, and is that
owner's destructor guaranteed to run on every exit path?

Counts as a violation:
- `new` with no matching `delete` on some reachable path
- two objects that both delete the same pointer
- a raw pointer stored past the lifetime of what it points into — especially pointers
  into FW_GrowingArray elements, which move when the array grows
- a component holding a pointer to an Entity that can outlive it
- an owning raw pointer passed across an API boundary with no documented transfer of
  ownership
- a container of owning pointers whose destructor does not delete the elements

Does NOT count:
- allocations intentionally living until process exit — singletons reached via
  GetInstance() are engine-lifetime by design per CLAUDE.md
- FW_GrowingArray / FW_String managing their own internal buffer
- non-owning back-pointers clearly scoped shorter than their target
- vendored SFML / ImGui internals

## Triage — run these first, read only the hits

Every finding for this lens requires an allocation, a raw pointer member, or a container
of pointers. Nothing else can host one. Set `S` first:

    S="Solution/Engine Solution/Framework Solution/ActionGame Solution/BossMonster Solution/TopDownGame"

1. Allocation and release sites, with file counts (~49 and ~12 files):

       rg -c '\bnew\b'    $S -g '!imgui' -g '*.cpp' -g '*.h'
       rg -c '\bdelete\b' $S -g '!imgui' -g '*.cpp' -g '*.h'

   **The gap between these two lists is your primary signal.** A file that allocates and
   never deletes, and whose type has no destructor doing it, is the shape you are hunting.

2. Raw pointer members (~55 headers) and containers of pointers (~26 files):

       rg -n '^\s*\w[\w:<>, ]*\*\s*my\w+\s*(=|;)' $S -g '!imgui' -g '*.h'
       rg -n 'FW_GrowingArray<[^>]*\*'            $S -g '!imgui'

3. Destructors, so you can check whether owners actually free (`~Type`):

       rg -n '~\w+\s*\(\s*\)' $S -g '!imgui'

4. Pointers taken into array elements — the FW_GrowingArray reallocation trap:

       rg -n '&\w+\[|\.GetLast\(\)|GetElement\(' $S -g '!imgui' -g '*.cpp'

Rank the union of these hits by how likely they are to host a real ownership bug —
files that allocate but never delete first, then containers of owning pointers, then
plain pointer members. Read the top files in batched `Read` calls, 5–10 per message.
Files with no hit in any of these greps cannot contain a finding for this lens; do not
open them.

Read `FW_GrowingArray.h` in full regardless — its growth and copy semantics are the
premise behind half the possible findings here, and you need them exact.
