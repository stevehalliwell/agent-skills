# Source model contract

Live schemas and usage are authoritative for actual fields. Distribution templates are starting contracts, not instructions to overwrite local schemas. No separate database or collection per theory.

## Owners

- `nouns`, `verbs`: entity/action identity and meaning.
- `interactions`: one conditional case; applicable `nouns`, one `verb`, separate `context_nouns`, coverage, shared-rule links, case consequences/feedback in body.
- `mechanics`: shared rules/state transitions; refer to controls for amounts and resources for pools.
- `systems`: constructed mechanic arrangements. `dynamics`: intended/predicted/observed behavior with explicit evidence status. `aesthetics`: intended player experience; links are not proof of feelings.
- `skill-atoms`: prerequisite AND sets, nullable signed authored rank, learning loop/understanding/expression/evidence sought.
- `challenges`: `entry`, `next`, `introduces`, `practises`, `requires`, `tests_mastery`, applicable design links, assistance/route context. `levels`: optional grouping only.
- `controls`: `value_type`, literal `value` or formula `expression`, units, enum values/bounds. `measures`: quantity meaning/type/unit/boundaries, not result values.
- `expectations`: declared measurable criterion; separate from simulation exit or design verdict.
- `experiments`: one question/method/snapshot and evidence links. `hypotheses`: claims/assumptions. `observations`: actual source/date/context/data, not generated predictions.
- `decisions`: every alternative and useful rationale, proposal/decision kind, common alternative group, subjects/related decisions, disposition, canonical adoption links.
- `glossary`: globally unique canonical term/aliases and inline meaning **or** documentation target. `resources`: extend noun with initial/capacity control references and ownership/lifetime; do not define a duplicate entity or behavior rule.

Templates: [all 19 collection contracts](../templates/). Each has `.schema.md`, `.usage.md`, `.template.md`. Stable `id`, technical `name`, and `created_at` belong to Attendant; mutable glossary terms do not replace record identity. References use configured `alias/name`, not file location.

## Partial design and lifecycle

Lifecycle: `proposed`, `accepted`, `rejected`, `superseded`, `defunct`. Supplied non-tentative facts may be accepted; generated options remain proposed. Accepted intent is not empirical truth. Keep inactive records/rationale; selecting one alternative leaves siblings proposed unless user states otherwise.

Missing/null/empty relationship arrays mean no declared links. Scalar null is not zero, false, empty string, or an inferred default. Optional absence permits discussion. Supplied malformed data, broken references, and conflicting definitions are not valid absence.

An accepted case may explicitly be `unknown`, `excluded`, or `not_applicable`; lifecycle and coverage answer different questions. A defined blocked action with no state change is `defined`, not missing. Case lists do not assert exhaustiveness. Do not manufacture noun–verb combinations as rules or defects.

At most one accepted resource extension per noun. Flow direction remains rule prose; no directional resource graph is inferred. Rank is conceptual placement, not physical level or prerequisite depth; preserve null/negative/zero. Declared challenge routes do not prove reachability or acquired skill.

## Types and unavailable semantics

Attendant schemas are flat. `null` declarations are optional/untyped, not invented integer markers. Intended rank/control/measure/expectation scalar subtypes need manual inspection in this release; generic `validate` **does not enforce these semantic types**. It reports performed checks and deferred C99 coverage. No fake `ref:any` field exists; generic documentation/subject strings require deliberate source interpretation.

Intended numeric contract: binary64 numbers; exact integers within ±9,007,199,254,740,991. Controls use integer/number/boolean/enum. Formula notation examples are authored data only: `${controls/walk-duration}` references, opaque case-sensitive unit symbols, explicit conversions, fixed functions. Do not `eval` formula strings, send them to a shell, or pretend this release resolves them. Ambiguous/invalid declarations stay unresolved until authorized correction.

Audience, capability, and motivation remain common concepts in prose/hypotheses. They are not mandatory cohorts/profiles or computed psychological scores. Add supplemental collection only for a distinct fact with no adequate existing owner and a reviewed setup/migration need.
