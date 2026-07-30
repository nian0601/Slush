---
name: plan-issue-with-grill
description: This skill should be used when the user asks to "plan an issue", "plan out a task", "break this down into an issue", "let's plan this before creating an issue", wants to think through a Trello card before it becomes a GitHub issue, or otherwise wants to scope a piece of work for the Slush repo (nian0601/Slush) before it gets filed. Runs an exhaustive clarifying-question pass and never assumes or infers scope, priority, or project.
argument-hint: [short description of the task]
allowed-tools: AskUserQuestion, Read, Grep, Glob, Skill
---

# Plan an issue before filing it

Turn a rough task description into a fully-scoped GitHub issue draft, through exhaustive clarification with the user, then hand off to the `create-issue` skill once — and only once — the user explicitly confirms the draft.

## Core rule: never assume or infer

Ask. Do not guess scope, priority, project, affected files, or acceptance criteria from the initial description, even when a guess seems obvious. The purpose of this skill is to reach a genuinely solid shared understanding before anything is written to GitHub — a wrong assumption here becomes a wrong issue that `/implement-issue` later executes against without noticing.

## Flow

If the skill was started as a hand-over from a grilling-session, then go to step 3 and use the information from the hand-over as 'grilling-result'

1. **Get the raw description.** Use `$ARGUMENTS` if given; otherwise ask what the task is.

2. **Interview the user** Do a grilling-session based on the task-description. Make sure that questions about verification is raised
   - **Verification**: how to tell each phase, and the issue as a whole, is actually done (build passes, specific manual test, `SkipStartScreen` playtest, etc. — see CLAUDE.md's Testing section for the conventions already established).

   While interviewing, watch for terminology drift: conflicting uses of the same word, unclear/ambiguous language, or a concept that keeps getting re-described without ever being given a specific name. Check `.claude/terminology.md` for an existing definition first. If a concept genuinely needs a new term, propose a name and definition and confirm it with the user before adding it to `.claude/terminology.md` — never add a term unilaterally.

3. **Break grilling-result into phases**, following the phased-work convention already in CLAUDE.md: small, independently buildable/testable chunks. For **each** phase, write enough detail that `/implement-issue` can act on it directly later without re-deriving context or asking the user to re-explain — not a one-line title. Capture, per phase:
   - What changes, and why.
   - The specific files/systems involved, as far as already known from the codebase check in step 2.
   - The approach/design decision that applies to this phase, and why, if a non-obvious alternative was rejected during clarification.
   - How to verify the phase is done.

   A phase entry should read like a subsection of an implementation plan, not a changelog line.

4. **Assemble the full draft**: title, one-paragraph summary, priority label, project label, and the phase-by-phase breakdown from step 4. Make a holistic judgment call on the size label, the same way priority is judged rather than mechanically derived: look at the assembled phase breakdown and estimate realistic wall-clock implementation time, then bucket it against CLAUDE.md's Issue tracking section anchors. This label is the source of truth — `create-issue` carries it through as-is rather than recomputing it.

5. **Show the complete draft to the user and ask for explicit go-ahead** before doing anything else. If the user wants changes, revise in place and show the updated draft again — don't hand off a stale version. Do not proceed on an implicit "looks fine" inferred from tone; get a clear confirmation.

6. **On confirmation, hand off**: use the Skill tool to load the `create-issue` skill. The confirmed draft is already in this conversation's context — `create-issue` should use it directly and must not re-ask anything already confirmed here.
