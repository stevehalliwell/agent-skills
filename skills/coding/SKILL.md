---
name: coding
description: "Use before any code change: writing, modifying, generating, patching, or deleting code. Load as soon as code changes are needed, including when implementation, web-implementation, or another delivery skill applies. Mandatory engineering layer for small, traced, proportionately verified changes. Skip recommendation-only work and documentation-only edits with no code change."
---

# Coding

Make the smallest correct code change after understanding the real path. Prefer code never written over code that adds no needed behavior.

This skill is the mandatory engineering layer for every code change. Other delivery skills may add workflow requirements; they do not replace or relax this one. Apply guidance throughout tracing, approach selection, editing, and verification, even when work becomes uncertain, large, or familiar.

## Workflow

1. Understand the change. Trace the affected path, callers, existing patterns, and relevant edge cases before proposing or editing code. Done when the real change point is known.
2. Choose the smallest solution. Apply the Ponytail ladder below. Prefer the approach that leaves difficult future work understandable, not merely the quickest or most familiar to write. Done when the chosen approach is the simplest correct option and each new abstraction removes current complexity or repetition.
3. Change and clean up. Implement requested scope, remove obsolete local references created by replacement, and mark deliberate constrained simplifications with a `ponytail:` comment. Done when requested behavior is complete without speculative scaffolding.
4. Verify proportionately. Run the smallest credible check for non-trivial logic; for bug fixes, prefer a reproduction that fails before the fix and passes after it. Correct in-scope failures and rerun affected checks. If correction is blocked or requires broader scope, report the blocker. State unrun validation and remaining risk without claiming a pass. Done when verification matches the change's risk and unresolved failures are explicit.

## Ponytail: minimum necessary code

### Solution ladder

Stop at the first rung that holds. Do not descend because a lower rung feels more familiar, configurable, reusable, or impressive.

1. **Does this need to exist?** Skip speculative needs. Do not build for a possible future requirement.
2. **Does the codebase already have it?** Look for an existing helper, utility, type, pattern, or boundary. Reuse it when it fits.
3. **Does the standard library do it?** Use it. If unsure, search documentation or ask for help.
4. **Does a native platform feature cover it?** Prefer it over custom code or a dependency: a database constraint over application-only validation, CSS over JavaScript, or a platform input control over a custom widget.
5. **Does an installed dependency solve it?** Use it. Never add a dependency for work that needs only a few correct lines.
6. **Can it be one direct expression or statement?** Write it directly.
7. **Only then:** write the minimum custom code that works.

The ladder is a reflex, not a research project or substitute for understanding. Read relevant code and trace the actual path end to end first. If two rungs work, take the higher rung. A small change in the wrong place creates another bug.

### Bug fixes

- A feature adds behavior; a bug fix restores promised behavior. A report or reproduction identifies a symptom, not necessarily the cause or correct change point.
- Trace the failing path and relevant callers. Fix the root cause rather than adding a guard only where failure appeared.
- Check similar usages of the same code. When callers share faulty behavior, fix the common cause once rather than repeating patches at known callers.
- Preserve behavior outside the defect. Do not bundle an unrelated refactor into the repair.
- Add the smallest credible regression check for a non-trivial defect. A reproduction that fails before the fix and passes after it is stronger evidence than a test that only exercises the new path.

### Restraint

- Add a helper, type, wrapper, interface, module, or configuration point only when it removes current repetition or entanglement. A possible future use is not a reason.
- Avoid an interface with one implementation, factory for one product, adapter for one call site, wrapper that only renames a direct operation, or configuration for a value that never changes unless it meets that present need.
- Do not add boilerplate or scaffolding for later. Add it when a real requirement creates it.
- Keep one-off transformations at their use site. Construct or return the needed value directly; do not stack conversion layers unless a current shared boundary requires them.
- Prefer deletion and clear, direct code over addition and clever code. Reduce concepts and dependencies, not file count at the cost of coupling independent concerns.
- Among equivalent standard-library options, choose correctness on relevant edge cases over brevity or familiarity.
- Do not add frameworks, fixtures, broad test suites, or dependencies unless the task needs them or the user requests them.
- Mark a deliberate corner cut with a known ceiling and upgrade path using a `ponytail:` comment. Example: `# ponytail: global lock; use per-account locks if throughput matters`.

## Simple versus easy

Build software that stays simple so difficult work remains possible. Optimize the artifact people must understand, debug, and change, not authoring convenience.

- **Simple:** unentangled. A change needs few independent concepts, layers, files, and hidden dependencies held in mind.
- **Complex:** entangled. Concerns that could vary independently are braided together; changing one requires understanding or changing others.
- **Easy:** familiar, near at hand, or within current capability. Relative to the developer; not evidence of simplicity.

### Design checks

- Before adding a layer, identify currently entangled concerns it separates. Keep it only when it makes a current change easier to reason about. Do not merely hide coupling behind a name.
- Prefer values and plain data for information. Keep mutable state narrow, explicit, and away from code that does not need time-dependent behavior.
- Define boundaries around independent concerns. Make dependencies and data flow visible. Separate policy, representation, timing, location, identity, and implementation when the task needs them to vary independently.
- Choose tools and patterns for resulting coupling, operational behavior, and edge cases, not familiarity or brevity.
- Fewer lines, files, objects, functions, or components do not prove simplicity. Do not split code just to shorten functions or make it look organized.
- Make code understandable by ordinary reasoning first. Tests, types, review, refactoring tools, and code organization are safety nets, not substitutes for local understanding.
- This is not a blanket ban on objects, methods, conditionals, loops, queues, ORMs, inheritance, or other constructs. Assess coupling in this codebase and task.

Before editing, check:

1. Does this make difficult future work more understandable, or only this edit more convenient?
2. Does the design expose essential complexity so it can be understood and changed, or obscure it behind extra layers?
3. What independent concerns does each new layer separate today?
4. Could a developer make the next likely change while holding fewer concepts and files in mind?
5. Is a direct value or plain data representation enough?

Omit a new layer without a clear present-tense reduction in entanglement.

## Required work, performance, and tests

Apply these checks when performance, dispatch, concurrency, or test strategy affects the change. They are design checks, not universal rules.

- Start with work the computer must perform. Write the simplest code that does it; do not force the work through a preferred pattern or style. Make data flow and cost visible.
- Split code at real conceptual boundaries. Use meaningful names that let readers work with an idea without reconstructing its details.
- Give repeated computations or domain operations one named implementation, such as `euclidean_distance`. Do not create tiny functions to satisfy a length rule; a split must clarify an operation or remove current duplication.
- When behavior is fixed or known at compile time, prefer direct, statically resolvable calls. Compiler visibility can enable inlining, simplification across boundaries, elimination of intermediate work, and vectorization.
- Runtime selection does not itself require virtual dispatch. Prefer a flat struct or tagged union with direct control flow when it makes the path visible. Use runtime polymorphism when implementation cannot be known or determined at compile time, not merely to follow a style rule.
- Account for lost optimization across an indirect call path, not only dispatch overhead. Consider the actual compiler, runtime, and call path; measure when performance claims materially affect a change.
- Seek readable, maintainable, sufficiently fast code. Hardware-specific specialization needs a real requirement proving ordinary direct code insufficient. Preserve a path to optimize later.
- Defer optimization of an isolated operation only when it can be replaced without changing surrounding architecture. Do not ignore performance in architectural choices that distribute cost across the system.
- Start independent requests or computations together; process results when available. Sequence only when a later step needs an earlier result. False serial dependencies set a latency floor that local optimization cannot remove.
- Choose tests by total engineering cost: writing, maintenance, constraints on legitimate changes, and bugs detected or prevented. TDD may fit a project; testing is an engineering tool, not a default driver of every design.
- Non-trivial logic—a branch, loop, parser, or money/security path—needs the smallest runnable check that would fail if it broke. An assertion-based demo, focused self-check, or small test can suffice. Trivial direct changes need no ceremony.

## Module boundaries and interfaces

Apply these checks when introducing or changing a module boundary or caller-facing interface. Working behavior matters, but repeated shortcuts can make later changes costly.

- **Problem decomposition:** parts that can be solved relatively independently. Assess how much each part must know about others, not how many classes or methods it creates.
- **Deep module:** substantial useful functionality behind a small interface. Interface cost includes every fact callers must know, not only signatures.
- **Shallow module:** little functionality relative to interface knowledge, or an interface that exposes most implementation details.
- **Strategic programming:** deliver behavior while keeping design understandable and changeable. **Tactical programming:** get the next change working while leaving complexity behind.

### Boundary checks

- Hide meaningful complexity behind a simple caller-facing interface. Divide complicated implementation internally rather than exposing its pieces to every caller. Unix file I/O illustrates a small public surface over complex internals.
- Design interfaces around component operations, not one client's actions. A text store can expose insertion at a position and deletion of a range; UI code can implement backspace, delete key, cursor, and selection using those operations.
- Build only needed functionality with a somewhat general-purpose interface: component concepts, not speculative features or universal reuse.
- Prefer an ordinary path covering related cases over a branch or API for every variation. Preserve genuinely different behavior and necessary error handling.
- Watch for shallow wrappers, tiny interfaces, client-specific APIs or state, information leakage, repeated special cases, and accumulating shortcuts.
- Do not make callers assemble several objects for an ordinary operation when a simpler interface safely covers it. Do not collapse genuinely separate concerns just to reduce object count.
- Do not impose fixed method or class line limits. Split at useful conceptual boundaries or to manage implementation complexity.
- Design in small steps. Document and revise as evidence changes. Consider the design you would choose had the new requirement been known from the start; improve simplicity as it reveals itself.
- Balance cleanup against scope, compatibility, and regression risk. The narrowest diff may add another costly exception. Do not justify complexity by assuming later technical-debt cleanup.

Before changing a boundary, check:

1. What can callers accomplish, and what must they know?
2. Does the boundary hide meaningful complexity or only rename a direct operation?
3. Does the interface reflect component concepts or leak one client's concerns?
4. Can an ordinary path cover these cases without losing distinctions or safeguards?
5. Will the next likely change be easier to understand, or only this edit faster?

## Code shape

- Extract repeated values, values with domain meaning, and values defined by a specification into descriptive constants or enums. Use the named protocol/specification constant when one exists.
- Keep a self-explanatory one-off literal inline when a named constant adds indirection without meaning. A constant is not configuration: do not create mutable configuration for a value that never varies.
- Prefer independent guard clauses over compound wrapper conditions to keep the main path shallow. Keep value returns and cleanup explicit; do not force early exits where they obscure cleanup, transaction boundaries, or normal flow.
- Name operations by their actual behavior; reserve threading-coordination terms for real synchronization, not immutable commitment or publication.
- Prefer an enum or equivalent named mode when a parameter selects behavior. Avoid boolean parameters whose `true`/`false` meaning is unclear at the call site; retain a boolean for a clear predicate or binary state.
- Separate logical blocks with a blank line: setup, validation, main work, and result/cleanup. Do not add whitespace mechanically between every statement.

## Rules

- Preserve requested scope. Apply a more specific workflow skill alongside this layer when its trigger fits.
- Reduce incidental complexity, never required protection, correctness, or understanding. Trace every relevant file and actual data/control flow before choosing a solution.
- Preserve trust-boundary validation; do not replace it with assumptions, local guards, or client-side checks.
- Preserve error handling that prevents data loss, corruption, or partial unsafe results. Do not suppress errors to make a path look uniform.
- Preserve security controls, authorization, secrets handling, safe defaults, accessibility basics, requested behavior, and compatibility.
- Do not choose the shortest implementation when it fails edge cases, obscures cleanup or transactions, or makes behavior harder to reason about.
- Physical-world and hardware integrations need calibration when real measurements require it. Do not remove necessary tuning merely because an ideal model needs fewer lines.

## References

Source attribution and optional background only. All operational guidance is inline; no source read is required.

- Rich Hickey, [Simple Made Easy transcript](https://github.com/matthiasn/talk-transcripts/blob/master/Hickey_Rich/SimpleMadeEasy.md): simple versus easy, independent concerns, plain data, and visible complexity.
- Casey Muratori, [Why most developers don't care about performance software (and should we?)](https://www.youtube.com/watch?v=8xBJPa_480Q): required machine work, direct calls, optimizable architecture, concurrency, and test cost.
- John Ousterhout, [software design talk](https://www.youtube.com/watch?v=LtRWu9DErgU): deep modules, caller knowledge, component-centered interfaces, and strategic design.
- Dietrich Gebert, [Ponytail](https://github.com/DietrichGebert/ponytail): solution ladder, minimum necessary code, tracing before editing, and verification without cutting safeguards.
