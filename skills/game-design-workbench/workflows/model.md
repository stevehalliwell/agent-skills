# Source model contract

Live schemas and usage are authoritative for actual fields. Shipped template files are authoritative starting contracts: copy/adapt them rather than regenerate them from code. They are not instructions to overwrite local schemas. No separate database or collection per theory.

## Owners

- `nouns`, `verbs`: entity/action identity and meaning. Noun `axes` explicitly references applicable typed dimensions; do not infer applicability from rule mentions.
- `axes`: independent enum/integer/number definitions, open named-state vocabularies by default, units and optional literal bounds. No per-state files, initial/current actor values, mandatory role taxonomy or implicit reachability. See [typed axis contract](../native/axes.md).
- `interactions`: conditional interaction record; applicable `nouns`, one `verb`, separate `context_nouns`, coverage and shared-rule links. One labelled `gd-rule` body block owns unordered named cases, participant bindings, conditions and effects; prose explains intent.
- `mechanics`: shared rules/state transitions; refer to control variables for configuration values and resources for pools.
- `systems`: constructed mechanic arrangements. `dynamics`: intended/predicted/observed behavior with explicit evidence status. `aesthetics`: individual intended feelings/experiential qualities at MDA's aesthetics layer. Name these directly; MDA category labels are optional. Use [shared emotion vocabulary](../references/emotional-experience/emotions.md) and [framing guidance](emotional-experience.md) when useful.
- `skill-atoms`: learned understanding and contextual applications, prerequisite AND combinations, nullable signed authored rank, learning loop/understanding/expression/evidence sought. Different methods may share a game rule; derive unlocks from prerequisites, not duplicate links. [Skill exploration](../native/README.md#skill-understanding-combinations-and-unlocks) uses explicit hypothetical knowledge, never automatic learning.
- `challenges`: `entry`, `next`, `introduces`, `practises`, `requires`, `tests_mastery`, applicable design links, assistance/route context. `levels`: optional grouping only.
- `control-vars`: named game configuration; `value_type`, literal `value` or formula `expression`, units, enum values/bounds. Documentation/simulations reference names rather than copying magic numbers. Shared C99 resolution, temporary overrides, exports, and read-only import plans are implemented; authorized source adoption uses public Attendant.
- `experiments`: all investigation data; no separate hypotheses, expectations, or observations collections. Body headings: question, hypotheses, local measures, expectations, method/assumptions, observations, results, interpretation, snapshot/provenance, limitations. Frontmatter field meanings belong to live usage.
- `glossary`: globally unique canonical term/aliases and inline meaning **or** documentation target. `resources`: extend noun with initial/capacity control references and ownership/lifetime; do not define a duplicate entity or behavior rule.

Templates: [all 15 collection contracts](../templates/). Each has `.schema.md`, `.usage.md`, `.template.md`. Stable `id`, technical `name`, and `created_at` belong to Attendant; mutable glossary terms do not replace record identity. References use configured `alias/name`, not file location.

## Actor/state rules

Read [rule reader](../native/rules.md) for body syntax and [typed axes](../native/axes.md) for definitions/applicability. Native `gd rules` currently reads declarations, validates axis types/units/bounds and resolves explicit noun/axis references, across all supplied statuses. It also binds declared effect state names and quantity targets through actor axes. [State conditions](../native/conditions.md) compile grouped AND/OR/NOT, explicit ANY and case-local skips, retaining unknown truth and participant scope. [Ordered context](../native/context.md) binds local participants and declared rule `context_axes`, preserving operand order without inferring contact or adding actor state. [Numeric quantities](../native/quantities.md) reuse shared control math. Optional [conditional outcomes/comparison](../native/outcomes.md) combines explicitly supplied starting/context assumptions, expands mapped invocations, resolves whole-case OTHER and checks simultaneous proposals. Bounded enum comparisons retain witnesses and unknown/unexamined scope; no numeric algebra/physics proof. Both shipped examples have full affected-rule migration; missing facts stay missing. Readiness flags are scoped facts, not gameplay validation. Never invent starting values, geometry, event timing, preservation or rules from omission.

## Experiments

Each experiment owns its measure names, types, units, and start/stop/undefined-value boundaries. No independent measures collection. Headings keep claims, criteria, actual observations, simulated outputs, and interpretation distinct within one investigation. Observations retain source/date/context and data links. Expectations retain measure, statistic, operator, thresholds, units, and pass-rate assumptions when supplied; currently body data, not executable threshold fields. No observation means no recorded evidence, not a rejected claim. Accepted hypothesis is still unverified without supporting evidence.

Agent generates one concrete test and records its evidence through [experiment workflow](experiment.md). For requested candidate comparison or control refinement, use [tuning extension](experiment-tuning.md). Interactive knobs or algorithmic search belong in experiment-specific setup, with temporary candidates and separate baseline adoption. No required machine-readable experiment fields or general expectation CLI.

Extend headings/fields as experiment implementation develops. Changed rules require a new simulation against committed code/inputs; historical results never follow live control variables. Do not fabricate executed trials, participants, results, or player observations.

## Partial design and lifecycle

Status is free text, default `proposed`; `accepted` selects active definitions in standard recipes. Users can retain ignored records under rejected/defunct/inactive/disabled or other chosen statuses, or request deletion. No compulsory alternative archive or workbench decisions collection; existing project-level decisions stay user-owned. Supplied non-tentative facts may be accepted; generated options remain proposed. Accepted intent is not empirical truth. Choosing one alternative does not change siblings without instruction.

Missing/null/empty relationship arrays mean no declared links. Noun applicability distinguishes omission/null (unspecified inventory) from `axes: []` (empty current inventory); neither proves complete design. Scalar null is not zero, false, empty string, or an inferred default. Optional absence permits discussion. Supplied malformed data, broken references, and conflicting definitions are not valid absence.

An accepted case may explicitly be `unknown`, `excluded`, or `not_applicable`; lifecycle and coverage answer different questions. A defined blocked action with no state change is `defined`, not missing. Case lists do not assert exhaustiveness. Do not manufacture noun–verb combinations as rules or defects.

At most one accepted resource extension per noun. Flow direction remains rule prose; no directional resource graph is inferred. Rank is conceptual placement, not physical level or prerequisite depth; preserve null/negative/zero. Declared challenge routes do not prove reachability or acquired skill.

## Types and unavailable semantics

Attendant schemas are flat. `null` declarations are optional/untyped, not invented integer markers. Generic `validate` **does not enforce all rank/control-variable/axis semantic types**. Native skill/control/rule consumers check their documented consumed types and operations; they do not replace live Attendant schema validation. State performed checks and unsupported coverage separately from Attendant output. No fake `ref:any` field exists; generic documentation/subject strings require deliberate source interpretation.

Intended numeric contract: binary64 numbers; exact integers within ±9,007,199,254,740,991. Control variables use integer/number/boolean/enum. Shared C99 control resolver consumes: `{control-vars/walk-duration}` references (no dollar prefix), opaque case-sensitive unit symbols, explicit conversions, fixed functions. Do not `eval` formula strings, send them to a shell, or duplicate evaluation in experiment code; use common resolver and document its coverage. Ambiguous/invalid declarations stay unresolved until authorized correction.

Audience, capability, and motivation remain common concepts in prose and experiment hypotheses. They are not mandatory cohorts/profiles or computed psychological scores. Add supplemental collection only for a distinct fact with no adequate existing owner and a reviewed setup/migration need.
