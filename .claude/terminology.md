# Terminology

Process/workflow vocabulary for this repo's Claude Code workflow (planning, grilling, issue tracking) — not codebase/engine domain terms, which stay documented where they already live (inline in code, `CLAUDE.md`'s existing sections).

This file is additive, not a migration target: well-established terms that already have a clean definition site (e.g. `no-plan`, the priority/size label taxonomy — see `CLAUDE.md`'s "Issue tracking" section) stay there. Only terms that don't already have an obvious home get captured here.

New entries require explicit user confirmation before being added — see the `grilling`/`plan-issue`/`plan-issue-with-grill` skills for when and how terms get flagged and proposed.

## Phase

A sub-step within a single `implement-issue` execution plan — the unit `plan-issue` breaks an issue into and `implement-issue` executes/commits one at a time.

**Not** a slice of a feature spanning multiple future planning cycles — see [Iteration](#iteration).

## Iteration

A slice of a feature's work spanning multiple future `plan-issue`/`implement-issue` cycles (each of which may itself contain several Phases). Used when scoping something bigger than one issue can hold.

**Not** a Phase — see [Phase](#phase).
