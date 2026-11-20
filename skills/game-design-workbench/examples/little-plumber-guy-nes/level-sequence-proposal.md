# Q14 — what opportunity annotations establish

Status: completed annotation exercise; no demonstrated level-design defect or recommended course change.

Correction after user review: two valid routes are not a problem. Final-gap component skills already have earlier declared opportunities on both routes. Only the exact composite label lacks earlier introduction/practice. That is not evidence that players are unprepared or another encounter is needed. Combining familiar skills for the first time may be intended.

Retained rehearsal below is a conditional illustration, not a proposed fix to pursue. Native results demonstrate effects of authored opportunity tags. Metadata-only control produces the same favorable coverage label without adding an encounter. Existing course remains unchanged; no defect, learning or quality conclusion follows.

## Brief and bounded question

[Concept](game-design/concept.md) asks players to finish by deliberately combining things they have learned. Focus only on checkpoint-to-finish sequence. Preserve fork, existing rule behavior, clock pressure and stateful checkpoint semantics. No new mechanic, control, schema, graphical review or playable experiment.

Unconfirmed premise used for this exercise: the exact combination should be rehearsed before its final test. This premise does not follow from the concept or native result. Practising components then combining them is not necessarily discovery of a new skill under pressure. Conditional question was where a rehearsal could sit without making upper route compulsory.

That **if** remains a designer choice. [Final-gap assessment](game-design/challenges/final-gap.md) leaves intent undecided. Missing prior composite introduction is a review prompt, not a diagnosis that the accepted course is bad or impossible.

## Source-backed baseline

- [Checkpoint strip](game-design/challenges/checkpoint-strip.md) precedes [stair fork](game-design/challenges/stair-fork.md).
- Fork offers [high stair](game-design/challenges/high-stair.md) or [low path](game-design/challenges/low-path.md); both lead directly to final gap, then [finish](game-design/challenges/finish.md).
- High stair declares extra landing practice; low path does not supply that particular encounter. Both remain legitimate route choices. “Longer” lower path has no measured duration or quantified advantage.
- [Finish approach](game-design/skill-atoms/finish-approach.md) combines [landing](game-design/skill-atoms/land-on-target.md) and [clock management](game-design/skill-atoms/manage-clock.md). Final gap tests that composite before any earlier explicit introduction/practice.
- Native baseline:19 challenges,18 demand/prerequisite checks. Final-gap composite support is `absent`; its five other checked skills/prerequisites have `all_routes` prior opportunities. This does not certify those skills were learned.
- Finish's later practice cannot support first arrival at final gap. Retry is stateful; do not invent ordinary back edges to manufacture support.

## Retained conditional illustration: shared post-fork rehearsal

Seven named encounters, with one of two fork branches visited:

1. **Checkpoint strip, unchanged.** Retain retry anchor. [Checkpoint rule](game-design/mechanics/checkpoint-save.md) grants no life, clock or power refill; trigger is invisible and feedback remains open. Do not claim player notices it.
2. **Stair fork, unchanged.** Keep choice of upper narrow-support passage or lower ground passage. Neither becomes a required lesson.
3. **High stair or low path, unchanged content.** Reroute both exits to proposed rehearsal instead of directly to final gap.
4. **Approach rehearsal, new proposal.** Broad approach strip with a visible, noninteractive launch cue. Ask player to jump onto a low broad step over continuous floor, settle landing, notice remaining clock and continue toward visible final approach. Intended missed-landing recovery stays on floor instead of falling into a lethal gap. No enemy or new finish trigger; no refill. Cue is layout prose, not an invented gameplay noun or interaction.
5. **Final gap, unchanged demand.** Remove continuous-floor recovery and ask for committed crossing. This is intended contrast between rehearsal and assessment, not a proved difficulty increment.
6. **Finish, unchanged.** Reach actual finish post after landing; rehearsal does not invoke course completion.

Private rehearsal declaration introduces finish-approach, practises landing/clock management and requires jump. It references existing jump-launch, solid-collision and active-clock mechanics plus step noun. No new skill rank or inferred player knowledge.

Desired contribution: make the combination legible before its final use while keeping branch choice. Expected player response is a hypothesis: some players may deliberately align approach, land and reconsider remaining time. Others may rush through, miss the cue, feel interrupted or lose useful clock. No response observed.

“Continuous floor” only proposes recovery from this landing mistake. [Active clock](game-design/mechanics/active-clock.md) still spends time and can expire. Exact geometry, collision, visibility and timing are unspecified; no claim that all forms/states can perform the sequence.

## Native counterfactual checks

Four isolated scopes use the same immutable skill definitions and separate challenge copies. Native `scaffolding` consumes all supplied statuses; isolation prevents proposal from being confused with accepted course. These are declaration queries, not game simulations.

- **Baseline:**19 challenges/18 checks; final-gap finish-approach support `absent`.
- **Upper-only control:** add introduction on high-stair only.19/18; `some_routes`. Native unsupported witness uses low-path and skips high-stair. Demonstrates why upper practice cannot be credited to every route.
- **Annotation-only control:** add introduction tag on checkpoint-strip, without new prose or encounter.19/18; `all_routes`. This deliberately demonstrates that native report trusts authored opportunity claims; a favorable label alone is not design evidence.
- **Shared rehearsal candidate:** reroute both branches through one proposed challenge.20/20; `all_routes`. Native supported witness passes low-path, rehearsal and final-gap; no unsupported witness for selected composite. Higher check count reflects new declared demand/prerequisite rows, not increased quality.

All four scopes have completed route/requirement analysis, zero omitted findings/witnesses and untruncated retained witnesses. Output retains one witness per relevant state, not every route enumeration. Proposed rehearsal stays `status: proposed`; successful query does not adopt it.

## Actual blockers and tool limits

1. **Physical realization absent.** Need step height/width, approach/landing extents, jump impulse/gravity, collision dimensions and sightlines. No tile map or executable course. Challenge links cannot establish reachability or intended recovery. Do not invent coordinates or add a physics planner merely to get a pass.
2. **Clock cost unknown.** Added encounter consumes active time. Arrival clock, route lengths, hesitation and retry costs are absent. [Travel-budget proposal](game-design/experiments/travel-budget.md) remains unexecuted and blocked on lengths; challenge counts are not seconds. Positive ideal travel margin would still not prove success.
3. **Practice claim needs evidence.** Native analysis cannot distinguish meaningful rehearsal from annotation-only control. Need observation of cue discovery, deliberate approach/landing, transfer at final gap and possible interruption/time loss. Do not infer intent from route choice or classify difficulty automatically.
4. **Final assessment intent undecided.** Keeping original composite discovery may be legitimate. Shared rehearsal changes pacing and clock cost; upper-only variant preserves an explicitly unsupported lower-route opportunity history. No option adopted by this exercise.
5. **Workflow friction, not demonstrated engine defect.** Counterfactuals require ordinary private copies because native query includes all supplied statuses. Source roles, exact field edits and prose are adequate for this bounded proposal. No demonstrated need for a new schema, metric or automatic designer.

Next evidence would require an explicitly scoped layout/playable prototype, then actual play observation. Neither authored nor run here. No experiment records/results changed; no simulated players or causal/quality verdict.

## Replay and source protection

From repository root, after building native CLI:

```powershell
node scripts/audit-workbench-level-sequence.mjs `
  --executable skills/game-design-workbench/native/build/reference-similarity-cli/gd.exe `
  --output skills/game-design-workbench/native/build/level-sequence-NEW
```

Output must be new, outside authored example, with existing parent. Script refuses reuse; emits one JSON receipt and retains native outputs, exact arguments, executable/script/source SHA-256 values and private fixture records in `manifest.json`/adjacent files. `--help` documents invocation; failure exits2. Native C owns route semantics; script makes copies and asserts expected results, not a second evaluator.

Final checked artifacts: `../../native/build/level-sequence-q14-2/`. Four native probes/assertion groups pass. Four command-contract checks pass: help, missing arguments, unknown option and existing-output refusal; refusal leaves prior manifest unchanged. Script syntax check passes. Non-correcting Attendant validation before schema preparation and strict validation after audit:225 records, no diagnostics/corrections. Authored game-design hashes unchanged across both audit runs.

No accepted record edit, adoption, viewer launch, player study, rule execution, commit or push. Private rehearsal is a native-query fixture, not an Attendant-created record. Adoption would need public record creation/identity, approved route edits and level membership. Level grouping remains unchanged. White-window and Q12/Q13 graphical review remain deferred.
