# John Ousterhout: software design

Build software that stays understandable as it grows. Working code is necessary, but repeated small shortcuts can make later changes costly. Apply these design checks with the coding skill's scope and safety rules, not as universal laws.

## Terms

-   **Problem decomposition:** Break a large problem into parts that can be solved relatively independently. Layers of abstraction are one way to do this; the important test is how much each part must know about the others.
-   **Deep module:** A class, method, or subsystem that supplies substantial useful functionality through a small interface. The interface includes every fact a caller must know, not just signatures. Functionality is the benefit; interface knowledge is the cost.
-   **Shallow module:** An abstraction whose interface reveals most of its implementation or offers too little functionality to justify another concept to learn.
-   **Strategic programming:** Deliver working behavior while keeping the design easy to understand and change. **Tactical programming** optimizes for getting the next change working, often leaving small amounts of complexity behind.

## Do

-   Decompose work around independent responsibilities. Assess a boundary by the knowledge it hides from callers, not by how many classes or methods it creates.
-   Give common operations simple interfaces, even when substantial implementation work is hidden behind them. The Unix file-I/O interface is an example of a small public surface over complex internals.
-   Keep a caller-facing interface simple; divide a complicated implementation internally when needed rather than exposing its pieces to every caller.
-   Design interfaces around underlying operations rather than one client’s actions. For example, a text store can expose insertion at a position and deletion of a range; the UI can implement backspace, delete key, cursor, and selection using those operations.
-   Build only the functionality needed now, while making its interface *somewhat* general purpose. Choose concepts that belong to the component, not speculative features for imagined clients.
-   Look for an ordinary path that handles several related cases without a branch or API for each variation. Preserve genuinely different behavior and necessary error handling.
-   Review for design red flags: shallow wrappers, too many tiny interfaces, client-specific APIs, information leakage across boundaries, repeated special cases, and small shortcuts that accumulate.
-   Design in small steps. Document and revise as you learn more; when changing existing code, consider the design you would choose if you had known the new requirement from the start.
-   Make improvements to simplicity as they reveal themselves.

## Do not

-   Do not impose a fixed line limit on methods or classes. Split at useful conceptual boundaries or to manage a complex implementation, not merely to make each piece shorter.
-   Do not introduce a method, class, or layer whose caller must understand almost its entire implementation to use it.
-   Do not make callers assemble several objects for an ordinary operation when a simpler interface can safely cover that common case. Conversely, do not collapse genuinely separate concerns just to reduce object count.
-   Do not confuse a somewhat general-purpose interface with building for hypothetical future requirements or making every module universally reusable.
-   Do not specialize a lower-level API around one client’s UI actions or expose client-specific state it need not know.
-   Do not assume the fewest changed lines produce the best change. A narrow patch that adds another exception to a poor design may cost more later; balance cleanup against scope, compatibility, and regression risk.
-   Do not suppress errors or remove required safeguards to make a path look uniform. The talk mentions “define errors out of existence” but does not explain it sufficiently to derive an error-handling rule here.
-   Do not justify added complexity by assuming a later technical-debt cleanup.

## Decision check

Before introducing or changing a boundary, ask:

1.  What can a caller accomplish through this interface, and what must they know to use it?
2.  Does the boundary hide meaningful complexity, or merely rename a direct operation?
3.  Does this interface reflect the component’s own concepts or leak one client’s concerns into it?
4.  Can one ordinary path handle these cases without losing required distinctions or safeguards?
5.  Will the next likely change be easier to understand, or are we only making this edit faster?

## Reference

-   [John Ousterhout, software design talk](https://www.youtube.com/watch?v=LtRWu9DErgU)