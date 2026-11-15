---
id: 01a16f86-6ff6-7e08-b993-4e47d0a7d09e
name: low-ceiling
created_at: 2026-10-24T20:07:23.894Z
desc: Overhead clearance rewards a deliberately shortened jump.
tags: []
status: accepted
entry: false
next:
  - challenges/flower-shelf
  - challenges/shell-lane
practises:
  - skill-atoms/shorten-jump
requires:
  - skill-atoms/jump
mechanics:
  - mechanics/jump-cut
  - mechanics/crouching
nouns:
  - nouns/ceiling
---
## Goal and context

A low overhead run constrains an otherwise familiar jump. Ground route remains available; shortened arcs can make reward access cleaner.

## Assistance and prior knowledge

Canopy introduced early release. Tall clearance has an unresolved stand-up case rather than a silently specified solver.

## Decision and assessment

Separate traversal from optional reward-taking. Ground route remains valid; not shortening a jump or skipping a reward is not automatically a failed encounter. No further performance-demand classification is established for ground traversal alone.

If player chooses a shortened-jump approach to a reward, accurate jump release is an execution task. Obtaining that chosen reward satisfies the local collection goal; missing it leaves that goal unmet without proving failed traversal, poor knowledge or an unintended input. Shortened arcs can make access cleaner, but no geometry establishes that they are the only successful technique. These are task-relative goals; no game-defined failure flag or wrong-answer feedback for a missed reward is declared.

Ascent release lowers remaining rise; exact cutoff law, heights and tolerances remain unknown. Tall/spark stand-up response under insufficient clearance is unresolved. Do not use an assumed safe correction or collision outcome as a success/failure criterion.

Intent check: should reward access invite optional execution practice without making shortened-jump performance compulsory for progress? Wider timing tolerance could be compared later, but no alternative or numeric window is adopted here. Prior practice opportunity is not proof of understanding; no reaction or dexterity classification follows from the action name.

```decision-assessment
id: ceiling-traversal
task: Continue through low-ceiling section.
context: Ground route remains available; skipping rewards or not shortening a jump is not automatically failure. Tall stand-up behavior under insufficient clearance is unresolved.
evidence_status: authored-assessment
rule_cases:
  - "mechanics/crouching#release-without-clearance"
demands: []
```

```decision-assessment
id: shortened-jump-reward
task: Attempt a chosen reward using a shortened jump.
context: Optional technique and collection goal, not compulsory traversal; exact geometry and timing tolerances are unknown, and missing reward does not identify cause of failure.
evidence_status: authored-assessment
rule_cases:
  - "mechanics/jump-cut#ascending-release"
demands:
  - kind: execution
    intent: undecided
```

## Route

Continue directly to shell-lane or take optional flower-shelf. Cellar return bypasses this whole area.
