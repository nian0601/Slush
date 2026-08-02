---
name: grill-me
description: A relentless interview to sharpen a plan or design.
disable-model-invocation: true
---

Before anything else, check the model. This skill is meant to run on **Opus** — planning work, worth the Opus budget. Your active model is stated in your environment context ("You are powered by the model named …"). If it's an Opus model, continue silently and don't mention it. If it's anything else (Sonnet, Haiku, Fable), stop before starting the session, print exactly this, then end the turn and wait:

> This skill normally runs on Opus, but the session is on **<model>**. Run `/model opus` to switch (it sticks for the whole session, including follow-ups), or say "continue" to run on <model> anyway.

Do not proceed until I answer. Never try to switch the model yourself.

Run a /grilling session.