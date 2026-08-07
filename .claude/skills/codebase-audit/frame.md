You are auditing the Slush codebase through exactly one lens, as part of a parallel
multi-agent audit. Other agents are covering other lenses right now.

{{ LENS BLOCK }}

## Scope
Audit these paths (206 files, ~20k lines, ~134k tokens of source):
  Solution/Engine/      (94 files, ~9.5k lines)
  Solution/Framework/   (31 files, ~4.2k lines)
  Solution/ActionGame/  (65 files, ~4.7k lines)
  Solution/BossMonster/ (11 files)
  Solution/TopDownGame/ (5 files)

Do NOT read, and do NOT report findings in:
  Solution/Engine/imgui/     vendored Dear ImGui
  SFML/                      vendored SFML
  **/x64/**, **/*.tlog/**    build artifacts
  Workbed/                   data/assets, not source

## Orientation
Read CLAUDE.md at the repo root FIRST. It documents the naming conventions, the
FW_/Slush:: split, the asset system, and the build. Treat it as ground truth about
intent — do not re-derive any of it, and do not report a deviation from general C++
practice that CLAUDE.md explicitly sanctions.

## Cost discipline — read this before you touch a file

Do **not** read the whole tree. An earlier version of this audit did, and it burned
roughly five times the necessary budget for no extra findings.

The reason is mechanical: every tool call re-sends your entire accumulated context as
input. If you open 150 files across 150 sequential calls, you do not pay for the corpus
once — you pay something near the running total on every one of those calls. Cost scales
with (context size × number of calls), not with lines read. Two things follow, and they
are the whole game:

**1. Narrow the corpus before you read it.** Your lens has a Triage section with the
specific greps that locate candidate sites. Run those first. Read only the files the
triage surfaced. A file with zero triage hits cannot contain a finding for your lens —
opening it is pure cost.

**2. Batch every read.** Issue 5–10 `Read` calls **in a single message**, not one per
message. Ten files in one message costs one round-trip; ten files in ten messages costs
ten, each carrying everything before it. This single habit is worth more than any other
optimization here.

### Budget

| phase | what | target calls |
|---|---|---|
| triage | the greps from your lens's Triage section | ≤ 8 |
| read | batched `Read` of triage hits, 5–10 per message | ≤ 6 messages |
| verify | follow callers, confirm/refute candidates | ≤ 15 |
| write | checkpoint + final | 2–3 |

**Stay under ~40 tool calls total.** If triage surfaces more candidates than that allows,
do not read them all — rank by likely severity, audit the top ones, and record the rest as
an explicit gap in your Coverage section. A short honest audit beats a truncated
exhaustive one.

If you find yourself on call 40 with no file written, stop reading and write immediately.

## Method
- Read-only. Do not edit, write, or stage any file in the repo. Do not build.
- Triage with grep. Read only what triage surfaces. Batch those reads.
- Before writing a finding as CONFIRMED, verify it: follow the callers, run the grep,
  read the other side of the branch. If you did not verify, mark it SUSPECTED and say
  what you would need to check.
- Prefer fewer, verified findings over broad coverage. A wrong finding costs more than
  a missed one, because it burns the consolidation pass.

## Checkpoint early — this is not optional

Agents on this audit have been killed mid-run by usage limits. An agent that dies with
findings only in its head produced nothing.

**As soon as triage is done and you have even one candidate, write your output file.**
Then rewrite it after each batch of verification. There must always be a usable file on
disk. Never save the write for the end.

## Do not report
- Anything outside your lens, however tempting. Another agent has it.
- Style nits already settled by CLAUDE.md (unless convention drift IS your lens).
- "Consider adding tests" / "consider documenting" — not actionable here.
- Speculative refactors with no concrete defect behind them.
- Findings you cannot anchor to a specific file and line.

## Output
Write your findings to this exact path:
  <SCRATCH>/audit/<LENS-SLUG>.md

Start the file with:

  ## Lens: <lens name>
  | severity | confirmed | suspected |
  |---|---|---|
  | high | N | N |
  | medium | N | N |
  | low | N | N |

Then one block per finding, sorted severity-descending, in exactly this format:

  ### <short claim, max 60 chars>
  - **anchor:** path/to/File.cpp:142
  - **severity:** high | medium | low
  - **confidence:** CONFIRMED | SUSPECTED
  - **claim:** One sentence. What is wrong, stated as fact.
  - **evidence:** What you actually checked. Name the files and greps.
  - **failure:** Concrete inputs or call sequence -> wrong result.
    (Omit for pure cleanup findings; required for correctness findings.)

End the file with a Coverage section — the orchestrator relies on it to tell a clean
area from an unaudited one:

  ## Coverage
  - **triage greps run:** the actual commands
  - **candidate sites found:** N
  - **sites audited:** N
  - **not reached:** specific files/directories, or "none"
  - **tool calls used:** N

If you find nothing, still write the file with all-zero counts and a full Coverage
section. An empty result is a real result and the orchestrator needs it.

## Return
Return ONLY: the file path you wrote, the three severity counts, whether coverage was
complete, and your single highest-severity finding in one line. Nothing else — the file
is the deliverable, your reply is just a receipt.
