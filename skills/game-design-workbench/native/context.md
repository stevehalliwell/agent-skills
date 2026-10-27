# Ordered interaction context

Shared C99 binds one authored context predicate to case-local participants and an explicit rule vocabulary. Raw body text stays authoritative. Names and operand order do not prove contact, geometry, symmetry, reachability or scene truth.

## Source and ownership

````markdown
---
context_axes: [axes/Contact]
---
```gd-rule
case contact:
    context: Player sideContacts Walker
    Player:
        noun: nouns/hero
    Walker:
        noun: nouns/walker
```
````

`axes/Contact` uses existing axis contract: `value_type: enum`, known `allowed_values: [sideContacts]`, optional `closed: true`. No context-specific definition schema, separate collection or mandatory role taxonomy.

- Interaction/mechanic `context_axes` explicitly selects context definitions for its named cases. Noun `axes` selects actor state. Loading a definition alone creates neither applicability nor contact.
- Omitted/null `context_axes` leaves vocabulary scope unknown; `[]` explicitly declares an empty current inventory. Preserve order and duplicates. Repeating an axis/value does not create another independent match.
- `context_nouns` remains ordinary related-noun links. It does not declare operators or bind case-local participants.
- Reusing a definition in context never adds it to actor state. An author may explicitly select the same definition for another use; analysis does not infer that selection.

## Body fragment

- `context: LEFT OPERATOR RIGHT` is ordered. LEFT/RIGHT must exactly name participants declared in this case. Same-type participants remain distinct. Names from another case or noun-reference spelling do not bind implicitly.
- Keep `A touches B` distinct from `B touches A`. Self-relations are retained; no physical impossibility or reflexivity rule is invented.
- Bare operator resolves only among the rule's declared context axes. Use `AXIS.VALUE` or `ALIAS/AXIS.VALUE` for disambiguation. Axis/value spelling is exact and case-sensitive.
- Canonical qualification may select a known applicable definition despite other unloaded references; it never creates missing inventory membership or searches global actor axes.
- `context: ANY` explicitly leaves context unrestricted. ANY is ASCII case-insensitive. Omitted `context` remains unspecified, not ANY or false; no vocabulary or participants manufactured for it.
- This slice accepts one triple or ANY, separated by spaces/tabs. Compound context logic and numeric relations are not implemented. Invalid triples retain raw text with a sourced error; no valid-looking partial relation is returned.
- Operator atoms are literal spans, not quoted strings/function calls. Qualification follows existing first-period state syntax. No new verb/input action is inferred from the operator.

## Headless output

Use existing `gd rules` with explicit interaction/mechanic, noun and axis mappings. See [rule reader](rules.md) for invocation and field selection.

- `data.records`: `context_axes_declared`, `context_axes_complete`, `context_axes_line`; existing sourced `relationships` includes explicit context-axis references. These are inventory facts, not vocabulary validity or scene truth.
- Each case retains raw `context`, `context_line`, plus `ordered_context`: syntax `state`, `unrestricted`, `bindings_complete`, local `left`/`right`, snapshot `left_index`/`right_index`, raw `operator` span/byte offset and canonical `target`.
- Endpoint indices are global participant indices within this rule-result snapshot. Unresolved endpoints use `-1`; names are null. No cross-reload identity implied.
- `target` retains axis/state and `resolved`, `undetermined`, `missing`, `ambiguous`, `wrong_type`, `invalid_definition` or `unavailable` resolution.
- Coverage adds `context_count` and `context_bindings_complete`. Readiness covers authored predicates only, not absent rules, concrete applicability or physical truth. ANY needs no vocabulary lookup. Zero authored contexts is not proof of an unrestricted design.

Missing/open information produces scoped informational findings, never false contact. Known empty inventories/closed-missing operators, ambiguous names, invalid definitions/types, syntax errors and undeclared endpoints produce sourced errors. Parsed syntax can still have incomplete/error bindings. No native source writes or automatic repair.

## Bounds and current example

- At most one context per existing bounded case: 4,096 cases, 8,192 participants, 8,192-byte body lines. Three token spans borrow retained rule text; canonical values borrow immutable model text. No allocation or recursion in context resolution.
- Local-name comparisons and operator lookup share existing 16,777,216 charged symbol-work budget with state predicates/effects. Exhaustion retains checked bindings and makes later targets unavailable, not missing. Metadata inventory references remain bounded by existing record/edge limits.
- Plumber's provisional `axes/Contact` preserves only the already-authored `sideContacts` value. The accepted powered-contact specimen explicitly selects it. Eight existing actor axes and noun inventories stay unchanged; other rules remain unmigrated.
- Tests cover order, same-type/self bindings, case-local identity, explicit/unknown/empty inventories, qualified/ambiguous/open/closed operators, duplicate references/values, wrong-role/type/definition paths, Unicode/raw offsets, metadata duplicate detection after field-index expansion, shared work exhaustion and source immutability. No scene or UX acceptance claimed.

[State predicates](conditions.md) remain separate logical facts. Resulting quantities, preservation/composition and bounded cross-file comparisons still need implementation before Review 01 completion.
