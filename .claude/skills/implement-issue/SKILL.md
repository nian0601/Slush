---
name: implement-issue
description: This skill should be used when the user asks to "work on issue #N", "implement issue 123", "start issue 123", "let's do this github issue", pastes a github.com/nian0601/Slush/issues/N URL, or wants to resume implementing a specific GitHub issue on the Slush repo.
argument-hint: [issue-number-or-url]
allowed-tools: Bash(gh:*), Agent, AskUserQuestion
---

# Work an issue on nian0601/Slush

Load a GitHub issue's phase breakdown and implement it phase by phase, per the repo's standing phased-work convention (see CLAUDE.md: small reviewable phases, stop after each for explicit review, never auto-chain to the next one).

## Steps

1. **Resolve the issue number** from `$1`/`$ARGUMENTS` or a URL pasted in the user's message.

2. **Load full context**:
   ```
   gh issue view <number> --repo nian0601/Slush --json title,body,labels,comments,url,state
   ```
   If the issue is already closed, confirm with the user before reopening/continuing rather than assuming it should be reopened.

3. **Parse the phase checklist** out of the body — each phase is a `- [ ]`/`- [x]` list item with its detail attached underneath (see the `create-issue` skill for the exact body format). Already-`[x]` phases are done; treat the first unchecked phase as the resume point. The issue body should carry enough detail (files, approach, verification) to start directly — if a phase genuinely lacks enough detail to act on, ask rather than guessing, but this should be rare since `plan-issue` is designed to prevent it.

4. **Work through remaining phases one at a time.** Implement a phase, verify it (build/run/whatever the phase's own verification note specifies), then **stop and wait for explicit review** before touching the next phase — this is a hard rule, not a suggestion, matching how every other phased change in this repo is handled.

5. **After a phase is reviewed and committed** (only if the user asked for the commit — same commit-approval rule as everywhere else in this repo), check its box in the issue body — this follows automatically from the commit approval, no separate go-ahead needed. Get the commit SHA (`git rev-parse --short HEAD`), fetch the current issue body, flip that phase's `- [ ]` to `- [x]` and append the short SHA in backticks to the end of the phase's title line (e.g. `- [x] **Phase 1: <title>** (`8e63edf`)`) so a later code-review pass can find exactly what changed for that phase, write it to a temp file, then:
   ```
   gh issue edit <number> --repo nian0601/Slush --body-file <tmpfile>
   ```

6. **Once every phase is checked off and committed**, run an automatic review pass before proposing to close the issue:
   - Prefer invoking the existing `code-review` skill if it can be given the issue's title/body as extra context for what the changes are supposed to accomplish (e.g. at `high` effort, without `--fix`).
   - If it can't take extra context that way, fall back to a direct `Agent` call with a code-reviewer-style prompt containing the issue's full text plus the actual diff (`git diff` against the pre-issue state), asking it to flag correctness problems specifically against what the issue asked for, reporting through `ReportFindings`.
   - Either way, this step **reports findings, it does not auto-fix them**. Surface them to the user.

7. **After findings are addressed (or waved off) and everything is committed, ask before closing**:
   ```
   gh issue close <number> --repo nian0601/Slush --comment "<short summary of what was done>"
   ```
   Same standing-permission model as a commit — never close without being asked in that turn.
