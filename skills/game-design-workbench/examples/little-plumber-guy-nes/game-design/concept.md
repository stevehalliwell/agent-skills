# Little plumber guy NES — opening course

Working mode: game-design

An authored fictional side-scrolling platform game. Familiar movement, blocks, pickups, pipes and a finish post make the example readable without a new setting. This is not a reconstruction, factual dataset or claim about the intentions of any existing game's creators. All constants are choices for this example.

## Experience and purpose

Let a player start by trying simple movement, become curious about rewards, and finish by deliberately combining a few things they have learned. The desired story is “I worked out what that thing does, tried a different approach, and made it across,” not merely “I moved right.”

The example also demonstrates how a familiar design can live as connected workbench records. That documentation purpose is separate from what should make playing enjoyable. No playable build or player study is included.

## Player and context

Single player, short opening course, digital directional input and two action buttons. Jump launches and can be released early. Action held requests running; a fresh Action press also requests a shot in spark form. Down crouches a powered form or enters a declared pipe when aligned. Exact input buffering, acceleration and collision timing remain open.

Intended player can try, notice feedback and retry; prior platform-game knowledge is not assumed proven. Familiarity, accessibility requirements and whether the opening teaches enough need play evidence.

## Player story

Hypothetical, not a participant quote:

“I moved away from the little creature, then found I could jump over it. The block above me gave me something that changed my size. Later, hitting a brick did something different. I followed some coins into a side room and came back farther along. Near the end I stopped rushing, lined up the last jump, and reached the post.”

A player could instead ignore rewards, lose power, avoid the shell, take the lower route or fail and retry. None of those alternatives is automatically an incorrect way to play.

## Core play

- [Movement](systems/locomotion.md): travel, jump from support, release early for a lower arc, read solid surfaces and commit across gaps. Camera follows forward progress and limits retreat.
- [Rewards](systems/reward-economy.md): bump supplies, collect coins or moving pickups, and notice changes. Coins can earn retries; score is feedback, not spendable money.
- [Power](systems/power-ladder.md): small, tall and spark forms change brick response, clearance and shooting. Enemy damage can remove capability; a brief grace window permits recovery but does not protect against falling.
- [Combat](systems/enemy-combat.md): direction and state matter. Descending top contact differs from side contact; a stationary shell can become a returning hazard.
- [Attempts](systems/attempt-pressure.md): lives limit lethal mistakes; a clock limits active play. A checkpoint reduces repeated approach work. Preserve distinction between retry budget and current power.

## Course over time

Opening apron offers safe movement before a lone walker. A conspicuous growth supply, mixed brick canopy and raised pipes add opportunities to try jumping, targeting and changed capabilities.

At the pipe section, continue toward the first gap and low ceiling or enter the optional [coin cellar](levels/coin-cellar.md). The cellar has safer reward gathering but shares the clock. Its one-way return skips the first gap, low ceiling and flower shelf. Skipping encounters does not prove missing understanding; it does change the opportunities the records can show.

All routes rejoin at shell-lane. Raised coins offer another voluntary detour. A safe checkpoint strip precedes a staircase fork: take upper landing practice or longer lower ground travel. Both reach the last gap and visible finish post.

[Challenge links](levels/opening-course.md) own this route. Level grouping is not another route order. No tile map, measured distance or physical reachability solver is supplied. Stateful retries return to an anchor through rules, not ordinary forward challenge arrows.

## Scope and boundaries

Only this opening overworld and its cellar are authored. No later worlds, swimming, moving platforms, boss, campaign economy or network play. No ROM, copied screenshots, map, sprites, external source capture or hardware-accurate physics.

[Named controls](control-vars/) own constants and simple travel-time formulas. Detailed rules and conditional cases remain in their own collections. Accepted records define this fictional example; proposed dynamics and aesthetics are explanations or intentions, not measured outcomes.

## Leads to investigate

- Can first-time players distinguish reward objects, enemy states and power loss? [Comprehension investigation](experiments/first-course-comprehension.md).
- Are optional routes discoverable, and does the clock support curiosity or suppress it? [Route-choice investigation](experiments/route-choice.md).
- Are intended travel budgets plausible once path lengths exist? [Travel-budget proposal](experiments/travel-budget.md), currently blocked on missing lengths.
- Does the last gap ask for a composite skill before enough support? Use authored prerequisite and route analysis, then play evidence; do not infer mastery from ranks.

Growth under low clearance, standing up under a ceiling, some shell interactions, and simultaneous death/finish ordering remain explicit design questions. They are not silently repaired by this example.
