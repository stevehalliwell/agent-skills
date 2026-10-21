# Save-Load Cycles — source notes

Independent research notes on one inspected catalog entry. Not an entry reproduction, book excerpt, or recommendation to change a game.

Actual source text: [complete local revision 26347 capture](captures/save-load-cycles-revision-26347.md). Capture method, checks, and local-only rights limits: [README](README.md).

## Source record

- Original title: *Save-Load Cycles*.
- Author attribution: Gameplay Design Patterns wiki, maintained by Staffan Björk and Jussi Holopainen. Individual revision author not verified. Entry identifies itself as an updated version of a pattern from their *Patterns in Game Design* (2004).
- Date: page footer says last edited 2018-03-18 at 12:55; timezone not stated.
- Type: author-maintained research wiki pattern entry.
- Original URL: https://gdp.cse.chalmers.se/index.php?title=Save-Load_Cycles
- Inspected revision: https://gdp.cse.chalmers.se/index.php?title=Save-Load_Cycles&oldid=26347
- Accessed: 2026-10-21.
- Method: web search discovery; `fetch_content` readable extraction of full entry and raw HTML inspection for revision, footer, status categories, and license evidence. Original analytical notes retained, not extracted source text.
- Reuse basis: unknown. Raw HTML/footer inspected; no content license statement found. No permission inferred from MediaWiki software or author's book citation. Full entry text and raw HTML now retained separately for private offline reference, not cleared for publication. Book text not acquired.
- Coverage limits: inspected entry, not linked game pages or related pattern entries. Source categories include “Needs revision”, “Needs references”, and “To be Published”. Some related concepts are redlinks. Treat relation claims as draft catalog analysis, not independently verified causal results.

## Relationship described

Relevant configuration involves returning to a stored game state after continuing play, potentially repeating this cycle to obtain a preferred outcome. Persistent storage alone is insufficient to establish the complete relationship: being able to resume a session and being able to revisit a decision serve different purposes. [Entry introduction; “Using the pattern”]

Technical restoration is only part of the relationship. The player may remember information from an abandoned attempt even though the restored world does not. That asymmetry can turn an uncertain event into a known problem on a later attempt. Entry connects this to experimentation, additional chances, and manipulation of results. [“Consequences”]

## Contextual trade-offs

- Interruption and retry solve different needs. Saving supports convenient breaks; reloading after an unwanted event also changes consequences. Do not collapse both into a single benefit. [“Consequences”]
- Timing restrictions matter. Save points constrain when a recoverable state becomes available; unrestricted save/load provides a different degree of control. [“Using the pattern”]
- Learning can become cheaper when a failed experiment can be undone. Entry also identifies loading downtime, so lower in-game risk need not mean no practical cost. [“Consequences”]
- Recovery can work against lasting penalties, irreversible events, narrative surprise, and tension. These are potential conflicts to inspect, not proof that all recovery removes all stakes. [“Using the pattern”; “Narration Aspects”; “Potentially Conflicting With”]
- Multiplayer restoration requires coordinating whose shared state is replaced. Entry contrasts this with persistent online worlds, where unilateral rollback conflicts with ongoing shared activity. [“Using the pattern”]

Catalog mentions ironman modes in the Europa Universalis and X-COM series as restrictions on these cycles. These are source-provided examples, not independently tested findings about every release or mode. [“Examples”]

## Why this entry is useful evidence

Entry provides a concrete instance of the framework's emphasis on interactions: state storage, restoration, retained player knowledge, permission timing, and lasting consequences jointly determine the result. It is not merely a label for any reversible action.

An in-world inverse move, a whole-state reload, and a bounded recovery action may differ in what they restore, what they cost, and what the player retains. This distinction is an original analytical reading, not a claim that all three are published patterns or equivalent implementations. No conclusion follows about whether a particular puzzle should allow pulling, undo, or another recovery rule.

The wiki links to “Reversibility” as a missing page in this revision. An attempted fetch of the alternate spelling “Reversability” returned 404. Neither a redlink nor a suggestive name provides an inspected definition. Use the concrete Save-Load Cycles evidence within its scope instead of inventing authority for a nonexistent local entry.
