---
name: systems-coding
description: "Use when writing, reviewing, designing, optimizing, or debugging C, C++, Zig, Rust, or other low-level systems code where memory, ownership, resource limits, latency, throughput, concurrency, hardware behavior, or failure handling matter. Apply TigerStyle-inspired ground rules for code that is bounded, predictable, safe, testable, and measured. Skip ordinary application code, mandated MISRA/AUTOSAR compliance, and formatting-only requests."
---

# Systems Coding

Ground rules for code worth keeping: explicit about its limits, honest about its cost, and designed for the machine that runs it.

## Applicability

Use these rules where resource behavior is part of correctness or product value. Project, platform, and regulated-domain rules take precedence. Do not turn a rule suited to a bounded-latency or safety-critical component into a universal restriction.

## Ground rules

- **Design for a stated operating model.** Know the inputs, lifetime and ownership of data, failure modes, and budgets for memory, CPU, storage, network, latency, and concurrency. Every material queue, loop, retry, buffer, allocation, and unit of work has an explicit bound or an intentional unbounded policy with backpressure or failure behavior.

- **Keep control flow finite and visible.** Prefer direct code with plain data and local state. Do not introduce recursion, hidden jumps, callbacks, retries, or asynchronous work that make ordering, progress, cancellation, or stack use unknowable. Keep a permanent event loop explicit and prove that it is intended not to terminate.

- **Make ownership and data movement local.** Show who owns, may mutate, and releases every resource. Use explicit allocators, not direct `malloc` calls; pass the allocator or caller-provided storage through the boundary that owns the allocation policy and lifetime. Avoid invisible allocation, mutable global state, stale aliases, and copies whose ownership or cost is unclear.

- **Treat memory bandwidth as a finite budget.** Use compact representations that match required data, layout, alignment, and protocol requirements. Do not initialize, retain, or move bytes an operation does not need. Avoid unnecessary `memcpy`, intermediate buffers, and pass-by-value copies of large objects: each consumes bandwidth and cache capacity, reducing attainable CPU throughput. Keep a copy when it establishes necessary ownership, isolation, layout, or lifetime safety.

- **Remove false serial dependencies.** Start independent work together, preserve data and control-flow independence, and add sequencing only where a result truly depends on an earlier result. This exposes parallel work and gives the compiler freedom to pipeline and schedule independent operations. When correctness permits, turn serial work into independent batches, partitions, or pipelines. Account for coordination, cache locality, data movement, and tail latency; added workers do not repair avoidable overhead or a serial bottleneck.

- **Make immutability and non-aliasing explicit.** Use `const` liberally for values and access paths that must not change; never cast away `const` to mutate or violate the underlying object's mutability contract. In C, use `restrict` liberally on pointer parameters only when their non-aliasing contract holds for every valid call, and document that contract at the API boundary. Do not apply it to potentially overlapping inputs. Where `restrict` is unavailable or prohibited, structure hot data and access paths to avoid unnecessary mutable aliases so the compiler can see independent work. `const` does not imply non-aliasing, and neither qualifier substitutes for checking ownership and lifetimes.

- **Measure hardware behavior, not conceptual performance.** Build and compare against a competent direct, single-threaded baseline before adding distribution or concurrency. Estimate the dominant costs, then measure representative hardware and workloads. Optimize algorithm, data layout, data movement, batching, and synchronization before micro-tuning. A performance claim names its workload, baseline, measurement, and limitation.

- **Separate expected failures from broken reasoning.** Handle or deliberately document operational errors where recovery belongs. Use side-effect-free assertions for violated programmer assumptions, invariants, and impossible states. Assert inputs, outputs, bounds, and critical relationships close to where they matter; test both the valid path and the boundary or failure path.

- **Use representations that make correctness checkable.** Prefer integers over floating point when the domain is discrete or exact arithmetic is required; choose widths, ranges, scaling, and overflow behavior explicitly. Floating point is fast on modern hardware, but integer is faster. Integer operations may be cheaper and some CPUs can execute integer and floating-point work in parallel. Neither advantage is universal: use floating point when the domain needs it, and measure representative workloads before changing a numeric representation for speed. Treat integer width, signedness, conversion, indexing, units, alignment, and byte order as design decisions when they affect behavior. Keep platform-specific code behind small boundaries with explicit contracts and fault models. Keep dependencies, macros, build variants, and feature flags few because each expands behavior that must be understood and tested.

## References

- [NASA/JPL Power of 10](references/10rules.md)
- [TigerStyle](references/TIGER_STYLE.md)
