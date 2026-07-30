---
name: diff-review
description: This skill should be used when the user explicitly asks to review code changes visually — "let's review this", "show me the diff", "I want to look at the changes side by side", or confirms (in the current turn) that they want a deep-dive code review, especially from a phone/tablet where GitHub Desktop isn't available. Renders a diff as a side-by-side/unified HTML page with collapsible per-file sections, published as an Artifact. Never invoke this proactively right after finishing a change — only on direct user request, or after the user has explicitly confirmed in this turn that they want one.
argument-hint: [optional: pathspec, ref/range, or "staged" — defaults to all uncommitted changes vs HEAD]
allowed-tools: Bash(git:*), Read, Write, Artifact
---

# Visual diff review

Renders a diff as a GitHub-Desktop-style side-by-side (and unified) HTML view, with collapsible per-file sections, published via the Artifact tool so it can be reviewed from any device — this exists specifically for reviewing code away from a desktop/GitHub Desktop.

## When to use — gate this carefully

- **Direct invocation** (`/diff-review`, typed by the user): always run.
- **Model-invoked**: only after the user has explicitly confirmed, in the current turn, that they want to visually review changes now. A finished implementation, a passing build, or a completed phase is **not** itself a trigger — do not offer or generate this automatically when work finishes. If it's ambiguous whether the user wants this versus a plain text summary, ask first rather than assume.

## Steps

1. **Determine scope.** If `$ARGUMENTS` is given, pass it through to `git diff` (e.g. a pathspec, `--staged`, a ref/range like `main..issue-42` or `HEAD~3`). Otherwise default to `git diff HEAD` (all uncommitted changes, staged + unstaged, against HEAD). If that's empty, say so and ask whether to review the most recent commit instead (`git show HEAD`) rather than silently picking something.

2. **Get the file list and stats**: `git diff --numstat <scope>` gives `<added>\t<removed>\t<path>` per file (added/removed show as `-` for binary files). Sum these for the top-bar summary (`N files · +X −Y`).

3. **For each changed file**, get its hunks: `git diff -U3 <scope> -- <path>`.
   - **Binary or pure rename** (no `@@` hunks — e.g. `Binary files a/... and b/... differ`, or just `rename from`/`rename to`): render the file card with its header only, plus a `<p class="filenote">` explaining why there's nothing to show (binary diff / renamed, no content change). Don't attempt to synthesize a diff grid for it.
   - **New or deleted file**: shows as one hunk with all `+` or all `-` lines (against `/dev/null`) — same row-building rules below apply, it's just one big add-only or del-only block.

4. **Build rows per hunk**, using the algorithm below, then render into `template.html`'s markup shape (see "Markup shape").

5. **Assemble the page**: start from `.claude/skills/diff-review/template.html`, fill in the placeholders (crumbs, title, summary stat, one `<details class="file" open>` block per file, footnote describing the scope), write the result to the scratchpad directory, and publish with the Artifact tool. Load the `artifact-design` skill first only if you're changing the visual design — reapplying this existing template doesn't need it. Reuse a stable filename (e.g. `diff-review.html`) within a session so re-running the skill later in the same conversation updates the same link instead of minting a new one every time; favicon `🔀`.

6. **Report the link back to the user** — one line, not a wall of text.

## Row-building algorithm

Walk each hunk's lines in order, tracking `oldNum` starting at the hunk's `-start` and `newNum` starting at its `+start`.

- A **context line** (prefix `' '`): one row, same escaped text on both sides, no background. Increment both counters.
- A **run of changes**: a contiguous block of `-` lines immediately followed by a contiguous block of `+` lines (either block may be empty/absent — e.g. an add-only or del-only run). Pair them positionally, GitHub-split-view style:
  - For `i` from `0` to `max(delCount, addCount) - 1`:
    - If `i < delCount`: old cell = del line `i`, red background (`num--del`/`code--del`), `oldNum++`.
    - Else: old cell = empty hatched placeholder (`num--empty`/`code--empty`), no number.
    - If `i < addCount`: new cell = add line `i`, green background (`num--add`/`code--add`), `newNum++`.
    - Else: new cell = empty hatched placeholder, no number.
  - This means an equal-count replace pairs up as modified-line rows (red left / green right, same row); a pure deletion run is all red-left/empty-right rows; a pure addition run is all empty-left/green-right rows; an uneven replace pairs what it can and spills the remainder into empty-paired rows.

For the **unified** rendering of the same hunk, don't pair — emit rows in original sequence order: context rows show both `oldNum`/`newNum`; a `-` line is one row (old number filled, new number cell empty, red background, `−` marker); a `+` line is one row (old number cell empty, new number filled, green background, `+` marker).

## Escaping

Line text must be HTML-escaped before insertion — code diffs routinely contain `<`, `>`, `&` (templates, comparisons, references): replace `&` → `&amp;` first, then `<` → `&lt;`, `>` → `&gt;`. Do not otherwise alter whitespace — the template's CSS uses `white-space: pre-wrap` (and `tab-size: 4`, matching this repo's tab indentation), so leading indentation and tabs render correctly as-is.

## Markup shape

Reference the already-built example structure below (this is exactly what step 4/5 produce — copy the pattern, not the content):

```html
<details class="file" open>
	<summary class="file__head">
		<div class="file__head-left">
			<span class="chev" aria-hidden="true">▸</span>
			<div class="file__path mono"><span class="dir">path/to/</span><span class="name">File.cpp</span></div>
		</div>
		<div class="file__stats"><span class="pill"><span class="add">+12</span><span class="del">−4</span></span></div>
	</summary>
	<div class="hunk__marker mono">@@ -40,7 +40,9 @@</div>
	<div class="diff diff--split mono">
		<!-- one row = 4 divs in sequence: num, code, num, code -->
	</div>
	<div class="diff diff--unified mono">
		<!-- one row = 3 divs in sequence: num, num, code -->
	</div>
	<!-- repeat hunk__marker + diff pair per hunk if the file has more than one -->
</details>
```

Multiple hunks in one file just repeat the `hunk__marker` + `diff diff--split` + `diff diff--unified` trio inside the same `<details>`.
