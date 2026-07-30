name: grilling
description: Grill the user relentlessly about a plan, decision, or idea. Use when the user wants to stress-test their thinking, or uses any 'grill' trigger phrases.

Interview me relentlessly about every aspect of this until we reach a shared understanding. Walk down each branch of the decision tree, resolving dependencies between decisions one-by-one. For each question, provide your recommended answer.

Ask the questions one at a time, waiting for feedback on each question before continuing. Asking multiple questions at once is bewildering.

If a fact can be found by exploring the environment (filesystem, tools, etc.), look it up rather than asking me. The decisions, though, are mine — put each one to me and wait for my answer.

While interviewing, watch for terminology drift: conflicting uses of the same word, unclear/ambiguous language, or a concept that keeps getting re-described without ever being given a specific name. Check `.claude/terminology.md` for an existing definition first. If a concept genuinely needs a new term, propose a name and definition and confirm it with me before adding it to `.claude/terminology.md` — never add a term unilaterally.

Do not act on it until I confirm we have reached a shared understanding. Once a shared understanding has been reached, ask if the resulting discussion and decisions should be passed on to /plan-issue-with-grill