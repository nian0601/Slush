---
name: implement-issue
description: This skill should be used when the user asks to "work on issue #N", "implement issue 123", "start issue 123", "let's do this github issue", pastes a github.com/nian0601/Slush/issues/N URL, or wants to resume implementing a specific GitHub issue on the Slush repo.
argument-hint: [issue-number-or-url]
allowed-tools: Bash(gh:*), Bash(git:*), Agent, AskUserQuestion
---

# Work an issue on nian0601/Slush (in an isolated worktree)

Implement a GitHub issue's phase breakdown phase by phase, directly in this session, per the repo's standing phased-work convention (see CLAUDE.md: small reviewable phases, stop after each for explicit review, never auto-chain to the next one). Do the work isolated in a git worktree on a transient `issue-<N>` branch (CLAUDE.md's standing convention), fast-forwarded back into `main` and deleted once the issue is done. Building and running the game inside the worktree doesn't collide with the primary checkout: `Directory.Build.props`'s `RepoRoot` resolves per-checkout, so each worktree gets its own `Workbed\`/`Build_Output\`.

If you want concurrent work on something else, run `/implement-issue` in a separate Claude Code session/instance — this skill doesn't manage concurrency itself.

## Steps

1. **Resolve the issue number** from `$1`/`$ARGUMENTS` or a URL pasted in the user's message.

2. **Load full context**:
   ```
   gh issue view <number> --repo nian0601/Slush --json title,body,labels,comments,url,state
   ```
   If the issue is already closed, confirm with the user before reopening/continuing rather than assuming it should be reopened.

3. **Create the worktree**:
   ```
   git worktree add .worktrees/issue-<N> -b issue-<N> main
   ```
   Nested *inside* the repo (`.worktrees/` is gitignored) rather than a sibling directory — the Bash tool's sandbox resets the persistent cwd back to the repo root if you `cd` outside the project directory, so a sibling worktree can't actually be worked in across separate Bash calls. Nesting it inside the repo keeps it within the sandbox boundary while `.worktrees/` in `.gitignore` keeps it out of the primary checkout's `git status`.

4. **Move into the worktree for the rest of this issue.** Resolve and remember the worktree's absolute path, then `cd` there via the Bash tool — its cwd persists across calls in this session (confirmed: this only holds while the path stays inside the repo tree), so every subsequent Bash call (build, `git add`/`commit`, `gh issue edit`, etc.) runs there implicitly with no repeated `cd`. Standing rule for the rest of this issue: Edit/Write/Read take absolute paths per call, not a persistent cwd, so **every** file edit must be prefixed with the worktree's absolute path (e.g. `<worktree-path>/Solution/...`) — never the primary checkout's copy of the same relative path.

5. **Work through phases one at a time:**
   - Parse the phase checklist out of the issue body — each phase is a `- [ ]`/`- [x]` list item with its detail attached underneath (see the `create-issue` skill for the exact body format). Already-`[x]` phases are done; treat the first unchecked phase as the resume point. If a phase genuinely lacks enough detail to act on, ask rather than guessing.
   - Implement the phase, verify it per that phase's own verification note (use the Bash tool, not PowerShell, for MSBuild/exe commands so you stay within the pre-approved permission rules), then **stop and report back to the user**. This is a hard rule, not a suggestion: never auto-chain to the next phase, and never commit without being told to.
   - Once told to commit a phase, commit it, then check its box in the issue body: get the commit SHA (`git rev-parse --short HEAD`), fetch the current issue body, flip that phase's `- [ ]` to `- [x]` and append the short SHA in backticks to the end of the phase's title line (e.g. `- [x] **Phase 1: <title>** (`8e63edf`)`), write it to a temp file, then `gh issue edit <N> --repo nian0601/Slush --body-file <tmpfile>`. Then move to the next phase and repeat.

6. **Once every phase is checked off and committed**, run an automatic review pass (still cwd'd in the worktree, so the diff is worktree-relative against the pre-issue base):
   - Prefer invoking the existing `code-review` skill with the issue's title/body as extra context (e.g. at `high` effort, without `--fix`).
   - If it can't take extra context that way, fall back to a direct `Agent` call with a code-reviewer-style prompt containing the issue's full text plus the actual diff (`git diff` against the pre-issue state), asking it to flag correctness problems specifically against what the issue asked for, reporting through `ReportFindings`.
   - This step **reports findings, it does not auto-fix them.** Report them and stop.

7. **Merge back to `main` and clean up** — do this in the primary checkout, not the worktree (`main` is checked out there, not in the worktree; git refuses to touch a branch checked out in another worktree from outside it):
   ```
   cd <primary-checkout-path>
   git merge --ff-only issue-<N>
   ```
   - On success: `git worktree remove <worktree-path>`, then `git branch -d issue-<N>`.
   - On failure (main diverged): `cd` back into the worktree, `git rebase main`, rebuild, and re-verify; if the rebase itself produces conflicts, **stop and ask** rather than resolving them silently. Once it's clean, `cd` back to the primary checkout and retry the merge.

8. **Ask before closing** (same standing-permission rule as a commit — never do this without being asked in that turn):
   ```
   gh issue close <number> --repo nian0601/Slush --comment "<short summary of what was done>"
   ```
