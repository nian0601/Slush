---
name: find-issue
description: This skill should be used when the user asks to "find an issue", "what should I work on", "find something that fits in X hours", "pick the highest priority issue", or states how much time is available this session and wants a recommendation from the Slush GitHub issue tracker (nian0601/Slush).
argument-hint: [time budget, e.g. 3h]
allowed-tools: Bash(gh:*)
---

# Find the best-fit open issue for the time available

Recommend an open issue that fits a stated time budget, weighted by priority — without silently starting work on it.

## Steps

1. Parse the time budget from `$ARGUMENTS`/`$1` or the user's message (e.g. "3h", "90 minutes", "most of the day").

2. List open issues:
   ```
   gh issue list --repo nian0601/Slush --state open --json number,title,labels,url
   ```

3. Sort by priority label, `priority:p0` first through `priority:p3` last. Also make a note of any issues with a 'no-plan'-label (to be reported in step 7)

4. Apply a **coarse** size pre-filter based on the time budget — sizes carry rough time anchors (see CLAUDE.md's Issue tracking section) but are still a single holistic guess made once with no feedback loop, so treat this as a shortlist heuristic, not a hard cutoff:
   - Small budget (roughly under 2h): prefer `size:s`.
   - Medium budget (roughly 2-6h): prefer `size:s` or `size:m`.
   - Large budget (6h+): any size is plausible; don't force a `size:l` pick over a smaller high-priority one that's also available.

   If nothing at the preferred size exists, widen the filter rather than reporting nothing.

5. For the top 2-3 shortlisted candidates (after the priority sort and size filter), read the full body to sanity-check the phase breakdown plausibly fits the stated budget:
   ```
   gh issue view <number> --repo nian0601/Slush --json body
   ```

6. Present the top candidate — number, title, and a one-line reason it was picked (priority + why the size looks like a fit). Present the next 3 best matching issues, regardless of reason why they were not chosen, in a short one-line format - number, title, priority and size. Dont list issues with the no-plan label here.

7. List all issues with the no-plan label, with the same short one-line format, but in a separate section.

8. **Ask the user to confirm before starting work.** Do not automatically invoke `/implement-issue`
