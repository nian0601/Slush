---
name: create-issue
description: This skill should be used when the user asks to "create a github issue", "create an issue for this", "file an issue", "add this to the tracker", or when a confirmed draft from the plan-issue skill needs to be written to GitHub. Creates issues on the Slush repo (nian0601/Slush) via the gh CLI with the priority/size/project label taxonomy, plus `no-plan` for phase-less issues.
argument-hint: [title] (omit when handing off from plan-issue)
allowed-tools: Bash(gh:*), AskUserQuestion, Write
---

# Create a GitHub issue on nian0601/Slush

Write an issue to GitHub with the correct labels and a body detailed enough that `/implement-issue` can act on it later without further clarification.

## Two entry paths

- **Handoff from `plan-issue`**: title, summary, priority, project, and a detailed phase-by-phase breakdown are already confirmed and present in the conversation. Use them as-is — do not re-ask anything already settled.
- **Direct/quick path** (`/create-issue` invoked on its own, for something too small to warrant the full planning process): work from `$ARGUMENTS`/the user's message. Ask for whatever's genuinely missing — **project is never defaulted, always ask if not stated**. Priority is the one field allowed a silent default on this path: use `priority:p2` if the user doesn't specify one. There is no phase list on this path unless the user gives one.

## Steps

1. Determine which path applies and gather the required fields: title, body summary, priority label, project label, optional phase list.

2. **Compute the size label** from the number of phases (thresholds in CLAUDE.md's Issue tracking section). No phase list → `size:s` unless the user said otherwise.

   **No phase list also means no plan**: add the `no-plan` label whenever the issue is being filed without a phase breakdown — whether that's the direct/quick path, or a deliberately-unplanned issue mentioned during a `plan-issue` handoff (e.g. a related future-work issue the user wants tracked but not scoped yet). If a phase list is present, do not add `no-plan`.

3. **Render the issue body.** Summary paragraph first, then one block per phase, each as a checkbox list item carrying its own detail so the phase stays individually checkable while keeping the context attached:

   ```markdown
   <summary paragraph>

   ## Phases

   - [ ] **Phase 1: <title>**

     <what changes and why, files/systems involved, approach/decisions, verification>

   - [ ] **Phase 2: <title>**

     <...>
   ```

   For a quick issue with no phases, just the summary paragraph is enough.

4. Write the body to a temp file (needed for multi-line `--body-file`), then run:

   ```
   gh issue create --repo nian0601/Slush --title "<title>" --body-file <tmpfile> --label "priority:pN" --label "size:X" --label "project:Y"
   ```

   Add `--label "no-plan"` too if this issue has no phase list (per step 2).

5. Report the created issue's number and URL back to the user.
