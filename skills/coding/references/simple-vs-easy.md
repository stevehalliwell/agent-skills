# Simple versus easy

Based on Rich Hickey's [Simple Made Easy](https://github.com/matthiasn/talk-transcripts/blob/master/Hickey_Rich/SimpleMadeEasy.md).

Build software that stays simple so difficult work remains possible. Do not optimize for making already-simple work easier to type, familiar, or immediately convenient.

## Terms

- **Simple**: unentangled. A change can be understood and made while holding few independent concepts, layers, files, and hidden dependencies in mind.
- **Complex**: entangled. Concerns that could vary independently are braided together, so changing one requires understanding or changing others.
- **Easy**: near at hand, familiar, or within current capability. It is relative to the developer and does not establish that the resulting system is simple.

Lines of code, number of files, and number of components are not measures of simplicity. Fewer lines can hide more coupling; several small, independent parts can be simpler than one tightly bound part. Layers and abstraction to not hide complexity in and of themselves, often they make more of it.

## Do

- Optimize for the artifact that must be understood, debugged, and changed over time, not for authoring convenience.
- Before adding a layer, identify which currently entangled concerns it separates. Keep it only when it makes a current change easier to reason about.
- Keep direct work direct. For a one-off transformation, construct or return the required value at the use site unless a real existing boundary requires otherwise.
- Prefer values and plain data for information. Keep mutable state narrow, explicit, and away from code that does not need time-dependent behavior.
- Define boundaries around one independent concern. Make dependencies and data flow visible at those boundaries.
- Keep policy, representation, timing, location, identity, and implementation separate when the task needs them to vary independently.
- Choose tools and patterns for their resulting coupling and operational behavior, including edge cases, rather than their familiarity or brevity.
- Use tests, types, and review as safety nets after making the code understandable by ordinary reasoning.

## Do not

- Do not add helpers, wrappers, adapters, types, interfaces, modules, or configuration just to make code look organized, reusable, or easier to extend later.
- Do not use a familiar construct as proof that it is simple.
- Do not treat fewer files, fewer objects, fewer functions, or fewer lines as proof of simplicity.
- Do not hide coupling behind an abstraction. Separate concerns; do not merely move their interdependence behind a name.
- Do not create a new abstraction for information that can remain plain data, or make a direct conversion travel through multiple layers without a current boundary.
- Do not rely on tests, types, refactoring tools, or code organization to compensate for code that cannot be understood locally.
- Do not apply this as a blanket ban on objects, methods, conditionals, loops, queues, ORMs, inheritance, or any other construct. Assess the coupling they create in this codebase and task.

## Decision check

Before editing, ask:

1. Does this change make difficult future work more understandable, or merely make this edit more convenient?
2. Does the design make the problem's essential complexity clear to see and manipulate, or does it add abstraction layers that make the goal appear easy while obscuring or increasing that complexity?
3. What independent concerns does each new layer separate today?
4. Could a developer make the next likely change while holding fewer concepts and files in mind?
5. Is a direct value or data representation enough here?

If a new layer does not produce a clear present-tense reduction in entanglement, omit it.
