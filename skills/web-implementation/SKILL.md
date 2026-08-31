---
name: web-implementation
description: "Use when implementing or reviewing web code, browser behavior, accessibility, responsive layouts, SEO/GEO, performance, or explicit web delivery requests. Layer standards-aware web constraints on agreed implementation work; use messaging-strategy for copy and technical-review for pre-code trade-offs. Delivery audit runs only when user explicitly asks to deliver, ship, or audit."
---

# Web implementation

Layer web-platform checks on the [implementation](../implementation/SKILL.md) workflow without duplicating its slicing, validation, or cleanup rules.

## Required read

For a web code change, load [Implementation](../implementation/SKILL.md) before inspecting implementation paths or editing code. It loads Coding's engineering constraints. Done when both layers apply before code-change work begins.

## Build

1. Identify changed web pages, browser/platform capabilities, and applicable accessibility, responsiveness, SEO/GEO, and performance constraints. Support major desktop, mobile, and tablet browsers from last two years. Done when relevant constraints are explicit.
2. Implement one agreed slice through the generic implementation workflow. Collect routine quality concerns for delivery review; flag direct material conflicts immediately with compliant alternatives. Done when slice preserves user control over material trade-offs.
3. Verify uncertain modern web guidance against authoritative current sources. Unless discussion-only, capture provenance through [OKF research artifacts](../references/okf-artifacts.md); ask for bundle root if unknown. Done when uncertain guidance is evidenced or deferred.

## Delivery review

Run only after explicit delivery language: deliver, ship, release, publish, audit, or equivalent.

1. Identify changed deliverable pages and applicable Lighthouse categories: Performance, Accessibility, Best Practices, and SEO. Target 100 in every applicable category. Done when pages, categories, and audit availability are known.
2. If audit tooling is absent, propose smallest setup and wait for approval before installing or changing project tooling. Done when audit can run or setup is explicitly deferred.
3. Fix every remediable issue within task outcome. Pause for behavior, visual, scope, external-dependency, or unsatisfiable-target trade-offs. Done when remaining issues require explicit user choice.
4. Review relevant browser, accessibility, and discoverability requirements; report scores, fixes, blockers, and follow-up. Done when delivery status is auditable.

## Rules

- Do not run broad audits during routine implementation slices.
- Keep website messaging and marketing strategist first for copy/positioning work.
- Do not install audit tooling without approval.
- Do not silently accept a non-100 applicable Lighthouse result or override material trade-offs.
