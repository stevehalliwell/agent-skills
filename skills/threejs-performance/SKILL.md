---
name: threejs-performance
description: "Use for explicit Three.js rendering, animation, GPU, frame-rate, memory, draw-call, shader, texture, or visual-performance work. Protect agreed render budget while keeping visual experiments reversible. Do not use for generic frontend changes or routine visual tweaks without stated performance concern."
---

# Three.js performance

Layer render constraints on the generic implementation workflow; retain its slice, cleanup, and reporting rules.

## Workflow

1. Establish budget. Before assessment or optimization without a project target, ask target device/browser/GPU class, FPS, resolution/device pixel ratio, representative scene and interaction complexity, memory, and network/load constraints. Done when budget is explicit.
2. Inspect render cost. Consider frame time, memory, draw calls, shader/texture cost, render passes/post-processing, resize/pixel ratio, animation frequency, and scene population. Done when likely cost drivers are known.
3. Flag material risk only. Before a request likely to materially raise per-frame work or memory, name concrete cost and propose focused measurement or lower-cost approach. Wait before profiling or changing requested visual direction. Done when user chooses path.
4. Keep experiments small and reversible. Use render evidence, not generic test suites, for visual performance work. Done when changed visual direction remains easy to compare or undo.
5. Handle tooling and guidance. If profiler/benchmark tooling is absent, propose smallest setup and wait before installation or project changes. Verify uncertain Three.js/browser guidance with authoritative sources and, unless discussion-only, use [OKF research artifacts](../references/okf-artifacts.md). Done when evidence is captured or deferred.

## Rules

- Explicit performance concern triggers this workflow; routine visual work stays quiet.
- Do not install profiling tooling without approval.
- Do not substitute broad tests for render measurements.
- Preserve user control over visual and performance trade-offs.
