# Crate and switch — working concept

Working mode: game-explorer

Authored tutorial brief, not a completed game or player-research finding. This gives exploration a candidate direction; it does not adopt every proposed mechanic or claim the intended experience has been achieved.

## Razor and purpose

A compact spatial puzzle about committing to crate moves: use limited push energy to put a crate on a switch and open the exit.

Purpose: explore how irreversible positioning, a finite action budget, and linked room states create planning decisions. The example also exercises workbench documentation and inspection; that tooling purpose is separate from what a player should enjoy.

## Intended player and appeal

Candidate player: someone who enjoys reading a small puzzle state, considering consequences, and trying a revised plan after a mistake. Appeal is understanding how a few interacting rules produce a solution, rather than fast execution.

Audience fit, accessibility needs, and assumed puzzle familiarity are untested. No market demand or demographic claim is implied.

## Intended experience

Aim for thoughtful commitment followed by the satisfaction of making a plan work. Player should understand what changed after an action and gradually connect crate placement, switch state, and available energy.

Local tension around a push can support that experience, but tension is not the whole goal. Explore whether limited energy creates useful planning or merely blocks experimentation. Do not treat faster completion as evidence of a better experience.

## Player story

Hypothetical player voice, not a participant quote:

“I thought putting the crate on the switch was the whole problem. Then I noticed that where I left it mattered for my next move, and I couldn't just pull it back. I restarted with a different plan. When the crate finally held the switch and the exit opened, the rules clicked together.”

This illustrates desired understanding and recovery through replanning. It is not an observed play sequence or a specified room layout.

## Play over time

### Second to second

Read nearby state, choose a walk or push, and inspect the result. A push commits position and spends energy when successful. Detailed conditions belong to the linked rules below.

### Minute to minute

Form a short plan for a challenge, inspect its consequences, and revise it. Restart is available as a rule, but where/how the player receives assistance or discovers restart remains unspecified.

### Hour to hour

No hour-scale loop, campaign, or retention system is defined. Current scope is a small set of connected challenge descriptions, not a promised long-form game.

## Core features and hook

- [Grid walking](mechanics/grid-walking.md) and [crate pushing](mechanics/crate-pushing.md) share a small spatial vocabulary.
- [Switch/exit coupling](mechanics/switch-exit.md) makes crate placement affect another part of the room.
- [Player energy](resources/player-energy.md) limits successful pushes. Values/formulas live in [named configuration](control-vars/), not this brief.
- [Restart](mechanics/challenge-restart.md) permits another attempt; challenge links and presentation still need investigation.
- [Challenges](challenges/) describe a possible progression with an optional practice route. Their skill-role labels are design intentions, not proof of teaching or mastery.

Hook to investigate: moving the crate solves one problem while constraining the next decision. Actual puzzle layouts must establish whether that produces interesting choices.

## Platform and context

Single-player interaction is assumed for this example. Target device, input scheme, presentation, engine, and delivery platform are not selected. Source records do not constitute a playable build.

## Constraints and risks

- No physical room geometry or executable rule model yet; route legality cannot be certified from these documents.
- Irreversible moves and scarce energy may create opaque dead ends rather than satisfying planning.
- Closed-exit walking behavior and some blocked-action feedback remain unresolved.
- A shortcut bypasses optional practice. Declared progression opportunities do not establish acquired knowledge on either route.
- Keep detailed rules, configuration values, and investigation evidence in their existing owners; this brief states direction, not a competing rules database.

## Open questions and emerging leads

- Does finite push energy encourage deliberate positioning, or discourage useful experimentation? See [planning investigation](experiments/energy-encourages-planning.md).
- What feedback makes blocked and successful actions understandable?
- Which assistance supports discovery without prescribing every move?
- Does local [deliberate tension](aesthetics/deliberate-tension.md) support or conflict with the broader intended satisfaction and willingness to retry?
- What can [route timing](experiments/route-cost.md) reveal under its stated assumptions? It is proposed, not executed, and does not model human puzzle solving.

Crate-color feedback remains proposed; pulling remains rejected in current source. Neither is silently adopted by this brief.

## Related products and commercial assumptions

No comparator study, business model, sales forecast, or market validation supplied. No commercial claim is needed to explore this tutorial design.
