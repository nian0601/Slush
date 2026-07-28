---
name: plan-issue
description: This skill should be used when the user asks to "plan an issue", "plan out a task", "break this down into an issue", "let's plan this before creating an issue", wants to think through a Trello card before it becomes a GitHub issue, or otherwise wants to scope a piece of work for the Slush repo (nian0601/Slush) before it gets filed. Runs an exhaustive clarifying-question pass and never assumes or infers scope, priority, or project.
argument-hint: [short description of the task]
allowed-tools: AskUserQuestion, Read, Grep, Glob, Skill
---

# Plan an issue before filing it

Turn a rough task description into a fully-scoped GitHub issue draft, through exhaustive clarification with the user, then hand off to the `create-issue` skill once — and only once — the user explicitly confirms the draft.

## Core rule: never assume or infer

Ask. Do not guess scope, priority, project, affected files, or acceptance criteria from the initial description, even when a guess seems obvious. The purpose of this skill is to reach a genuinely solid shared understanding before anything is written to GitHub — a wrong assumption here becomes a wrong issue that `/implement-issue` later executes against without noticing.

## Flow

1. **Get the raw description.** Use `$ARGUMENTS` if given; otherwise ask what the task is.

2. **Interview the user** Interview user relentlessly about every aspect of this plan until we reach a shared understanding. Walk down each branch of the design tree, resolving dependnecies between decisions one-by-one. If a question can be answered by exploring the codebase, explore the codebase instead. Ask the questions one by one, do not flood the user with all questions at once.

3. **Clarify exhaustively**, using `AskUserQuestion` (structured choices) or plain follow-up questions (open-ended points) as fits each question. Cover, at minimum:
   - **Goal and scope**: what problem this solves, what the end state looks like, explicit non-goals (what's deliberately out of scope).
   - **Priority** — always ask; offer `p0` (drop-everything/blocking) through `p3` (low/someday) with `p2` described as the normal default, but let the user pick. Never pick it silently.
   - **Project** — only ask if its not clear which project the plan belongs to: `project:engine` or `project:actiongame`.
   - **Approach/design decisions**, where more than one reasonable approach exists — surface the options and their trade-offs rather than picking one.
   - **Verification**: how to tell each phase, and the issue as a whole, is actually done (build passes, specific manual test, `SkipStartScreen` playtest, etc. — see CLAUDE.md's Testing section for the conventions already established).
   - Anything else that stays ambiguous after the above — keep asking rather than filling a gap with a plausible-sounding default.

4. **Break the work into phases**, following the phased-work convention already in CLAUDE.md: small, independently buildable/testable chunks, one thing per phase. For **each** phase, write enough detail that `/implement-issue` can act on it directly later without re-deriving context or asking the user to re-explain — not a one-line title. Capture, per phase:
   - What changes, and why.
   - The specific files/systems involved, as far as already known from the codebase check in step 2.
   - The approach/design decision that applies to this phase, and why, if a non-obvious alternative was rejected during clarification.
   - How to verify the phase is done.

   A phase entry should read like a subsection of an implementation plan, not a changelog line.

5. **Assemble the full draft**: title, one-paragraph summary, priority label, project label, and the phase-by-phase breakdown from step 4. Compute the size label from the phase count (`size:s` for fewer than 4 phases, `size:m` for 4-7, `size:l` for 8+) and include it for visibility, though `create-issue` recomputes it independently.

6. **Show the complete draft to the user and ask for explicit go-ahead** before doing anything else. If the user wants changes, revise in place and show the updated draft again — don't hand off a stale version. Do not proceed on an implicit "looks fine" inferred from tone; get a clear confirmation.

7. **On confirmation, hand off**: use the Skill tool to load the `create-issue` skill. The confirmed draft is already in this conversation's context — `create-issue` should use it directly and must not re-ask anything already confirmed here.
