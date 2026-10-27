---
id: 01a16f86-7801-7c00-8130-73e87a151064
name: first-course-comprehension
created_at: 2026-10-24T20:07:25.954Z
desc: Proposed playtest of action understanding and readable state changes.
tags: []
status: proposed
stage: proposed
experiment_kind: playtest
mechanics:
  - mechanics/jump-launch
  - mechanics/enemy-contact
  - mechanics/supply-release
  - mechanics/shell-travel
dynamics:
  - dynamics/learn-by-retry
  - dynamics/power-protection
  - dynamics/shell-risk
aesthetics:
  - aesthetics/playful-confidence
  - aesthetics/earned-competence
---
## Question

Can a first-time player distinguish movement, top/side enemy contact, reward collection and resting/sliding shell states?

## Hypothesis

Readable feedback and repeated related situations may support understanding. This is not tested.

## Local measures

- explained-contact: boolean, unitless, whether participant correctly predicts a supplied top/side contact case before trying it.
- first-shell-response: enum, unitless, avoid/launch/stop/other, first intentional response after noticing shell.
- retry-count: integer, attempt, retries before ending session; undefined with reason if observation is lost.

## Method and expectations

Proposed moderated playtest after a playable build exists. Record prior familiarity and actual prompts; ask participant to explain selected decisions without teaching answers first. Participant count, stopping rule and success criteria remain open; no invented threshold.

## Observations and results

None. No build, participants, generated program or execution exists. No design verdict.

## Provenance and limits

Authored investigation for this fictional example. Play evidence must be retained with consent and context; route support findings are not a substitute for comprehension observations.
