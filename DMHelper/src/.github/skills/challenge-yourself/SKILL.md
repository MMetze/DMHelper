---
name: challenge-yourself
description: Use when asked to challenge the current changes, review a diff, or pressure-test an implementation. Trigger phrase: "challenge yourself".
---

Review the current changes like a skeptical senior engineer.

## Focus
- Evaluate coding style and consistency with the existing codebase.
- Hunt for crash risks, lifetime bugs, threading issues, null dereferences, and resource leaks.
- Challenge assumptions, not just syntax: verify the likely runtime path and failure mode before judging a change.
- Call out unnecessary duplication, branching, abstraction, or complexity.
- Prefer the simplest correction that removes risk.
- Call out missing tests or validation when a change touches a crash-prone or concurrency-sensitive area.
- Treat local cpp namespaces as a style mismatch unless the surrounding codebase clearly uses them.

## Context handling
- Use the current diff and nearby implementation as the primary source of truth.
- If a Windows GitHub Desktop path matters, infer the current user profile from the path structure itself, for example by normalizing `C:\Users\<profile>\AppData\Local\GitHubDesktop\GitHubDesktop.exe` to the active user profile path. 

## Output
Return an executable correction plan, not a patch.

Format the plan as:
- High impact changes: items that reduce crash risk, data corruption, or major maintainability issues.
- Low impact changes: cleanup, style alignment, or small simplifications.

For each item include:
- what should change
- why it matters
- the smallest concrete action to take

Keep the plan concise, specific, and actionable. Prefer the fewest items needed to make the next correction obvious.