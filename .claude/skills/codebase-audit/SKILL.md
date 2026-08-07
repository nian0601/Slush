---
name: codebase-audit
description: This skill should be used when the user asks to "audit the codebase", "run a codebase audit", "do a multi-agent review of the whole codebase", "sweep the code for X", or otherwise wants a fan-out review where several agents each examine the entire Slush codebase through one lens and the findings are consolidated into a single ranked report. Not for reviewing a diff or a branch — use /code-review for that.
argument-hint: [lens names, or omit to run all four]
allowed-tools: Task, Agent, SendMessage, Read, Write, Glob, Grep, Bash
---

# Multi-agent codebase audit

Fan-out review of the Slush codebase. N agents, one lens each. Each agent triages the
whole tree with greps tuned to its lens, then reads only the sites that surface. Findings
land in a shared format so consolidation is mechanical rather than a re-read of N essays.

For reviewing pending changes or a PR, this is the wrong skill — use `/code-review`.

## Cost model — read before changing anything here

This skill previously consumed a full session usage limit on one run. The cause is worth
understanding, because the obvious fixes are the wrong ones.

Every tool call an agent makes re-sends that agent's entire accumulated context as input.
An agent that opens 150 files over 150 sequential calls does not pay for the corpus once;
it pays something near the running total on each call. **Cost scales with (context size ×
number of calls), not with lines read.** The corpus here is only ~134k tokens — reading it
is cheap. Reading it across 150 round-trips is not.

Two consequences, both now enforced in `frame.md`:

- **Triage before reading.** Each lens file carries verified greps that reduce 206 files to
  a few dozen candidates. Files with no triage hit cannot host a finding for that lens.
- **Batch every read.** 5–10 `Read` calls in one message, not one per message.

The evidence is in the run that prompted this: `depth-and-size` cost 77k tokens over 27
tool calls while `absence-handling` cost 315k over 150, on the identical tree. The only
difference was that `depth-and-size` already had a mechanical triage step and the others
were told to "read files in full". That instruction is gone.

Do not "fix" cost by cutting lenses or shrinking scope first — fix it by keeping triage
tight. A lens that greps well is nearly free.

## Why there is no shared "reader" agent

Subagents don't inherit context. A pre-pass agent that reads every file holds that
content in its own context window only; the sole thing it can pass onward is a summary,
and reviewers auditing prose about the code cannot find a lifetime bug in it.

Note this argument justifies each agent reading *independently* — it does not justify each
agent reading *exhaustively*. With triage, the lenses barely overlap in which files they
open, so the real cost is well under corpus × N. Independence is what makes cross-lens
agreement meaningful in step 4; exhaustiveness was never what bought that.

## Lenses

| Slug | Lens | Model | File |
|---|---|---|---|
| `ownership-lifetime` | Ownership & lifetime of allocations | opus | `lenses/ownership-lifetime.md` |
| `absence-handling` | Handling of absent/failed lookups | opus | `lenses/absence-handling.md` |
| `convention-drift` | Divergence from CLAUDE.md conventions | sonnet | `lenses/convention-drift.md` |
| `depth-and-size` | Oversized functions & shallow classes | sonnet | `lenses/depth-and-size.md` |

Opus is reserved for the two correctness lenses, where a missed defect is the expensive
outcome. `convention-drift` and `depth-and-size` are mechanical once triaged — their lens
files are prescriptive enough that Sonnet follows them reliably.

If the user named specific lenses in the arguments, run only those. Otherwise run all four.

Adding a lens later: write `lenses/<slug>.md` following the shape of the existing ones —
a yes/no question, an explicit "counts as a violation" list, and an explicit "does NOT
count" list. A lens without the negative list will meander.

## Procedure

### 1. Prepare

Create `<SCRATCH>/audit/`. Read `frame.md` and the file for each lens being run.

### 2. Spawn

**All agents in a single message**, so they run in parallel. Per agent:

- `description` — "Audit <short lens name>"
- `subagent_type` — `general-purpose`. Never `Explore`; it reads excerpts to locate
  code and does not audit.
- `model` — per the table above.
- `run_in_background` — `false`. Consolidation needs all of them and there's nothing
  useful to do meanwhile.
- `prompt` — the contents of `frame.md` with the lens file's contents substituted for
  the `{{ LENS BLOCK }}` marker, and `<SCRATCH>` / `<LENS-SLUG>` filled in.

Tell the user before spawning that this is a paid fan-out and roughly what it costs, so
they can cut it to two lenses if the budget is tight.

### 2a. If an agent gets killed

Usage limits and interruptions kill agents mid-run. `frame.md` requires each agent to
write its file as soon as triage produces a candidate, so **check `<SCRATCH>/audit/`
before assuming work was lost** — there is usually a usable checkpoint.

To continue a killed agent, resume it with `SendMessage` using the `agentId` from its
spawn result rather than spawning a fresh one; its read context survives in the transcript.
Be aware of the tradeoff: a resumed agent re-sends its full accumulated transcript on
every subsequent call, so it restarts at its most expensive per-call state. Resuming is
right when the agent was nearly done, and wrong when it had barely started — in that case
a fresh agent with good triage is cheaper than reviving a bloated transcript.

If the run died from a usage limit, do not immediately resume all agents in parallel.
Confirm with the user first.

### 3. Collect

Read every `<SCRATCH>/audit/*.md` in full. Do not work from the agents' returned
receipts — those are summaries, the detail is in the files.

Read each file's `## Coverage` section before its findings. A lens that audited 6 of 40
candidate sites has not cleared the other 34, and its silence about them is not evidence.

### 4. Consolidate

Build one flat list across all lenses, tagging each finding with its source lens.

**Group by anchor.** Two findings collide if they name the same file within ~20 lines
of each other, or describe the same defect in different words. Merge collided findings,
keeping the clearest claim and unioning the evidence.

**Re-score confidence, overriding what the agents self-reported.** An agent grading its
own work is unreliable; two agents reaching the same site from different lenses is real
evidence:

- flagged by 2+ lenses → CONFIRMED; if the lenses disagreed on severity, take the higher
- CONFIRMED by one lens, uncontested → stays CONFIRMED
- SUSPECTED by one lens, uncontested → **verify it yourself now.** Read the file, follow
  the callers. Promote or drop. Never pass a hunch through to the report.

**Spot-check the numbers.** Findings from `depth-and-size` carry counts (lines, public
methods, nesting depth). Verify a sample against the actual files — a wrong count means
that agent's whole file is suspect.

**Discard** anything unanchored, anything outside all running lenses' scope, and anything
in vendored `SFML/` or `Solution/Engine/imgui/` that slipped through.

### 5. Sanity-check the sweep

Before reporting, check the run rather than the code. Each lens file ends with a
`## Coverage` section giving its triage greps, candidate count, audited count, and what it
never reached. Use it:

- **audited ≪ candidates** → that lens sampled. Report the ratio; do not present its
  silence as a clean bill of health.
- **zero findings with full coverage** → a real result, report it as such.
- **zero findings with thin coverage** → the agent stopped early. Say so.
- **findings clustered in one directory** → check whether triage actually reached the
  others, or whether the agent ran out of budget partway through its candidate list.

Distinguishing "audited and clean" from "never looked" is the whole point of this step —
triage makes the sweep cheap, and the cost is that a lens can now silently cover less.

### 6. Report

Write `<SCRATCH>/audit/REPORT.md` using the template below. Then summarize in chat:
total counts, top 3 findings, and any coverage gap from step 5. Don't paste the whole
report into chat.

Do not fix anything. Do not commit. Propose next steps and stop.

## Report template

```
# Slush codebase audit — <date>

<N> lenses over ~20k lines of source. <M> findings after consolidation.

## Summary
| lens | high | medium | low | total |
|---|---|---|---|---|
| ... | | | | |
| **total** | | | | |

Cross-lens agreement: <K> findings flagged independently by 2+ lenses.

## Coverage
| lens | candidates | audited | not reached |
|---|---|---|---|
| ... | | | |

<Any lens where audited < candidates is a sampled lens, not a clean one. Say which.>

## High severity
<finding blocks, each noting which lens(es) found it>

## Medium severity
<finding blocks>

## Low severity
<one line each: anchor + claim>

## Dropped during consolidation
<SUSPECTED findings that failed verification, one line each with why.
 Kept so the next audit doesn't re-litigate them.>

## Proposed follow-up
Which findings should become GitHub issues on nian0601/Slush, with proposed title
and `project:` label. Grouped into batches that work as single work items rather
than one issue per finding.
```

## After the report

`REPORT.md` is a transient work product. Per the knowledge-routing rule in
`~/.claude/CLAUDE.md`, deferred work becomes a GitHub issue on `nian0601/Slush` with the
right `project:` label — not a line in a document that rots. Offer to file them; don't
file without being asked.
