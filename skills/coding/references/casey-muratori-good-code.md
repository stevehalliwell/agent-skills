# Casey Muratori: simple, good code

Adapted guidelines from Casey Muratori's interview, [Why most developers don't care about performance software (and should we?)](https://www.youtube.com/watch?v=8xBJPa_480Q). Timestamps point to source captions retrieved on 2026-09-14. These are design checks, not universal rules.

## Guidelines

### 1. Begin with required machine work

Identify what the computer must actually do to solve the problem, then write the simplest code that performs it. Do not begin with a design pattern, style rule, or preferred abstraction and force the work through it. Direct code makes the required work, data flow, and cost visible. [1:24:35–1:25:20]

### 2. Create digestible pieces with meaningful names

Break code at real conceptual boundaries, and name each piece for what it represents or does. Optimize for the person who must understand and modify it later, often its original author. A name earns its place when it lets readers work with the idea rather than repeatedly reconstructing its details. [1:24:52–1:25:10]

### 3. Remove real repetition; do not split code mechanically

Give a repeated computation or a domain operation one named implementation. For example, use `euclidean_distance` rather than reproduce its equation at every use site. Do not create tiny functions merely to meet a function-length rule: a split must clarify a real operation or remove current duplication. [1:18:19–1:18:58; 1:25:23–1:25:40]

### 4. Prefer compile-time-known, direct calls when behaviour is fixed

When the required operation is known at compile time, use a direct function or other statically resolvable call. The compiler can inline such calls, remove redundant intermediate work, combine adjacent operations, and sometimes vectorize or widen the resulting code.

Use runtime polymorphism only when the implementation cannot be known or determined at compile time. A requirement that the program select behavior at runtime does not itself require virtual dispatch: prefer a flat struct or tagged union with direct control flow when it can make the needed path visible. These representations retain known operations where a virtual inheritance hierarchy would hide them behind an indirect call.

Do not introduce runtime polymorphism only to follow a rule such as “always prefer polymorphism.” Virtual dispatch leaves open the possibility of a substituted implementation, which can prevent inlining and other whole-path optimizations. The important cost is often lost optimization across the path, not only the indirect call itself. Keep calls plain and direct when their behavior is fixed or can be determined at compile time. Complete visibility lets an optimizing compiler inline freely, simplify across call boundaries, and remove intermediate work. [1:18:24–1:21:43]

### 5. Seek readable, maintainable, sufficiently fast code

Good code usually lies at the intersection of code that runs well enough, is easy to read, and is easy to modify. Reaching a hardware-specific theoretical maximum may justify specialized, less-readable code, but only after a real requirement proves that ordinary direct code is not sufficient. Preserve a path to optimize later rather than prematurely specializing every path. [1:26:13–1:27:29]

### 6. Make architecture optimizable before deferring optimization

It is safe to defer optimizing an isolated operation when it can be replaced later without changing surrounding architecture. It is not safe to ignore performance while choosing architecture: some choices distribute a cost through the system and cannot be repaired as a local hotspot. Before deferring work, decide whether the boundary lets a later change target one implementation. [31:43–33:23]

### 7. Avoid false serial dependency chains

Start independent requests or computations together, then process their results when available. Add a sequence only when the later step truly requires the earlier result. A chain of unnecessary request-response steps sets a latency floor that concurrency, threading, or a later micro-optimization cannot remove without architectural rewrite. [33:39–36:14]

### 8. Choose tests from total engineering cost

Use tests when their cost to write and maintain is lower than the cost of bugs they detect or prevent. Include the cost of tests that constrain legitimate future changes, alongside the cost of defects escaping. Test-driven development can be the right project decision, but testing is a tool for engineering judgement rather than a default driver of every design. [1:22:07–1:24:26]

## Use

Apply these checks with the existing coding guidance. Preserve trust-boundary validation, correctness, security, accessibility, and requested behavior. Measure when performance claims materially affect a change; do not assume a particular abstraction helps or harms without considering its actual compiler, runtime, and call path.
