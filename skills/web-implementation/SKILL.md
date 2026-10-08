---
name: web-implementation
description: "Use when implementing or reviewing web code, browser behavior, accessibility, responsive layouts, SEO/GEO, performance, or explicit web delivery requests. Layer standards-aware web constraints on agreed implementation work; use messaging-strategy for copy and technical-review for pre-code trade-offs. Delivery audit runs only when user explicitly asks to deliver, ship, or audit."
---

# Web implementation

Layer web-platform checks on the [implementation](../implementation/SKILL.md) workflow without duplicating its slicing, validation, or cleanup rules.

## Trigger clarification

Start from the supplied web change or review target. Manual invocation without a target asks which page, browser behavior, or audit to work on. Route copy-only work to [Messaging strategy](../messaging-strategy/SKILL.md) and pre-code architecture choices to [Technical review](../technical-review/SKILL.md).

- `Fix mobile navigation`: implement and run targeted checks; no broad delivery audit.
- `Audit this landing page`: run delivery review; report findings without editing unless fixes are authorized.
- `Rewrite the headline`: use messaging-strategy, not this skill.

Done when target, implementation/review scope, and delivery-audit trigger are known.

## Required read

For a web code change, load [Implementation](../implementation/SKILL.md) before inspecting implementation paths or editing code. It loads Coding's engineering constraints. Done when both layers apply before code-change work begins.

## Build

1. Identify changed web pages, browser/platform capabilities, and applicable accessibility, responsiveness, SEO/GEO, and performance constraints. Support major desktop, mobile, and tablet browsers from last two years. Verify uncertain modern web guidance against authoritative current sources before relying on it; report unresolved guidance as a blocker when it affects correctness. Done when relevant constraints and supporting evidence are explicit.
2. Implement agreed work through the generic implementation workflow. Apply relevant web constraints during implementation; do not defer known in-scope defects to delivery review. Flag material conflicts with compliant alternatives. Done when requested behavior preserves user control over material trade-offs.
3. Run targeted checks for changed behavior and applicable constraints. Correct in-scope failures and recheck; report unavailable checks and blockers. Done when checks pass or unresolved failures are explicit.

## Delivery review

Run only after explicit delivery language: deliver, ship, release, publish, audit, or equivalent. Audit-only requests authorize inspection and reporting, not code changes. This review does not itself authorize publishing or deployment.

1. Identify requested or changed deliverable pages and applicable Lighthouse categories: Performance, Accessibility, Best Practices, and SEO. Target 100 in every applicable category. Record tested URL, device profile, tool version, and scores so reruns use comparable conditions. Done when pages, categories, audit conditions, and availability are known.
2. If audit tooling is absent, propose smallest setup and wait for approval before installing or changing project tooling. Done when audit can run or setup is explicitly deferred.
3. When fixes are authorized, load Implementation before code-change work, fix every remediable issue within task outcome, then rerun affected pages/categories. Continue while progress is possible within scope. Pause for material behavior, visual, scope, external-dependency, repeated no-progress, or unsatisfiable-target trade-offs; report remaining issue and needed decision rather than looping indefinitely. For audit-only work, record findings without editing. Done when applicable scores reach target, audit-only findings are recorded, or remaining failures need an explicit user choice.
4. Check relevant browser behavior, accessibility, and discoverability beyond Lighthouse scores; scores alone do not establish conformance. Report tested pages and conditions, scores, other checks, fixes, blockers, and checks not run. Do not claim delivery readiness while required checks are blocked or targets remain unmet. Done when delivery status is auditable.

## Rules

- Do not run broad audits during routine implementation slices.
- Route website copy and positioning to [Messaging strategy](../messaging-strategy/SKILL.md); this skill owns web-platform implementation and checks.
- Do not install audit tooling without approval.
- Do not silently accept a non-100 applicable Lighthouse result or override material trade-offs.
