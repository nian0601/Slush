---
name: implement-issue
description: This skill should be used when the user asks to "work on issue #N", "implement issue 123", "start issue 123", "let's do this github issue", pastes a github.com/nian0601/Slush/issues/N URL, or wants to resume implementing a specific GitHub issue on the Slush repo.
argument-hint: [issue-number-or-url]
allowed-tools: Bash(gh:*), Agent, AskUserQuestion, SendMessage
---

# Work an issue on nian0601/Slush (in an isolated worktree)

Load a GitHub issue's phase breakdown and implement it phase by phase, per the repo's standing phased-work convention (see CLAUDE.md: small reviewable phases, stop after each for explicit review, never auto-chain to the next one). The actual implementation runs in a background subagent inside its own git worktree (`isolation: "worktree"`), so this session stays free — you can plan other work, or run `/implement-issue` again for a different issue, while it runs. Building and running the game inside a worktree doesn't collide with the primary checkout: `Directory.Build.props`'s `RepoRoot` resolves per-checkout, so each worktree gets its own `Workbed\`/`Build_Output\`.

## Orchestrator steps (run in this session)

1. **Resolve the issue number** from `$1`/`$ARGUMENTS` or a URL pasted in the user's message.

2. **Load full context**:
   ```
   gh issue view <number> --repo nian0601/Slush --json title,body,labels,comments,url,state
   ```
   If the issue is already closed, confirm with the user before reopening/continuing rather than assuming it should be reopened.

3. **Spawn the worker**: call `Agent` with `isolation: "worktree"`, `subagent_type: general-purpose`, and a self-contained prompt built from the **Worker prompt template** below (substitute the issue number and paste the full `gh issue view` JSON in). Leave `run_in_background` at its default (background). Note the agent id the call returns — `SendMessage` needs it later.

4. **Relay phase reviews.** The worker stops after each phase and reports back. Each time a report arrives:
   - Surface it to the user exactly as if you'd done the work yourself inline.
   - Once the user responds (approves, asks for changes, says "commit it", etc.), relay that instruction to the worker via `SendMessage(to: <agent id>, ...)`. It acts on it (revises, commits, checks off the phase, moves to the next phase) and stops again.
   - Repeat until the worker reports that every phase is checked off and committed, and its automatic review pass (step 4 of the worker template) is done.

5. **Merge back to `main`** — do this here, in the primary checkout, never inside the worker's worktree (`main` is checked out here, not there; git refuses to touch a branch checked out in another worktree from outside it):
   ```
   git merge --ff-only issue-<N>
   ```
   - On success: remove the worktree (`git worktree remove <path>`, using the path the `Agent` call returned) and delete the branch (`git branch -d issue-<N>`).
   - On failure (main diverged — e.g. you committed something else directly in this session while the worker ran): `SendMessage` the worker to `git rebase main` inside its own worktree, rebuild, and re-verify; once it confirms, retry the `git merge --ff-only` here.

6. **Ask before closing** (same standing-permission rule as a commit — never do this without being asked in that turn):
   ```
   gh issue close <number> --repo nian0601/Slush --comment "<short summary of what was done>"
   ```

## Worker prompt template (embed verbatim in the spawned Agent's prompt)

> You are implementing GitHub issue #<N> on nian0601/Slush, running inside your own isolated git worktree of the Slush repo. Full issue JSON is below.
>
> **First action**: rename your branch for clarity: `git branch -m issue-<N>`.
>
> 1. **Parse the phase checklist** out of the issue body — each phase is a `- [ ]`/`- [x]` list item with its detail attached underneath (see the `create-issue` skill for the exact body format). Already-`[x]` phases are done; treat the first unchecked phase as the resume point. The issue body should carry enough detail (files, approach, verification) to start directly — if a phase genuinely lacks enough detail to act on, ask rather than guessing.
>
> 2. **Work through remaining phases one at a time.** Implement a phase, verify it (build/run/whatever the phase's own verification note specifies — use the Bash tool, not PowerShell, for MSBuild/exe commands so you stay within the pre-approved permission rules), then **stop and report back**. This is a hard rule, not a suggestion: never auto-chain to the next phase, and never commit without being told to.
>
> 3. **Once told to commit a phase**, commit it, then check its box in the issue body: get the commit SHA (`git rev-parse --short HEAD`), fetch the current issue body, flip that phase's `- [ ]` to `- [x]` and append the short SHA in backticks to the end of the phase's title line (e.g. `- [x] **Phase 1: <title>** (`8e63edf`)`), write it to a temp file, then `gh issue edit <N> --repo nian0601/Slush --body-file <tmpfile>`. Then move to the next phase and repeat from step 2.
>
> 4. **Once every phase is checked off and committed**, run an automatic review pass:
>    - Prefer invoking the existing `code-review` skill with the issue's title/body as extra context (e.g. at `high` effort, without `--fix`).
>    - If it can't take extra context that way, fall back to a direct `Agent` call with a code-reviewer-style prompt containing the issue's full text plus the actual diff (`git diff` against the pre-issue state), asking it to flag correctness problems specifically against what the issue asked for, reporting through `ReportFindings`.
>    - This step **reports findings, it does not auto-fix them.** Report them and stop.
>
> Do not merge into `main`, close the issue, or remove your own worktree — that all happens from the primary checkout once you've reported back.
>
> Full issue JSON:
> `<paste the gh issue view JSON here>`
