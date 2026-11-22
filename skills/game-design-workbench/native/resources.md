# Resource declarations and conditional witnesses

Use the **Resources** pane or `gd resources` to inspect resource roles, guards, deltas, and concrete mapped declarations. Require `--interactions` and/or `--mechanics`; bind nouns, axes, resource definitions, controls, and other related records as needed.

```sh
gd resources --mechanics 'mechanics=game-design/mechanics' \
  --nouns 'nouns=game-design/nouns' --axes 'axes=game-design/axes' \
  --resources 'resources=game-design/resources' --fields nodes,relations --json
```

Without endpoints this is an inventory. Source definitions and mapped placements are distinct. Look at whole guards and side declarations, not only the highlighted relation. Resource role prose can explain ownership or lifetime, but does not define a shared pool by itself. Only explicitly bound pool + axis identity joins local facets across cases. Shared noun types or local participant names do not join supplies.

## Conditional witnesses

Use `--to NODE_ID` to look upstream, or `--from NODE_ID --to NODE_ID` to inspect possible declared connections. Get exact node IDs from the same bound inventory or the viewer. `--from` alone is not a route query. `--accepted-only` excludes proposed sources; default eligibility includes accepted and proposed, and interactions must be defined. Other dispositions can remain visible in the inventory. `resource-routes` offers the same class of witness through [named queries](named-queries.md).

In Explorer, stage FROM/TO from selected nodes or leave FROM blank for upstream, then Run. Inspect witness steps, their whole guards, side declarations, and any frontiers or omissions. **Only witness + side declarations** limits display, not query scope.

- An enabling requirement is not consumption. A checked negative delta is a debit; a checked positive delta is a credit declaration, not a final award. Preservation, exclusions, and raw incidence are not production arcs.
- A directed witness can retain contradictory guards, alternative branches, mapped calls, and side costs. It does not establish starting supply, timing, applicability, compatible states, or an executable sequence.
- Read construction coverage and route limits separately. Unknown directions, unresolved bindings, and limited or cyclic expansions can leave open frontiers. No known witness does not prove impossibility.

For general structural questions, Resource adjacency is undirected **incidence**. An incidence path or bypass is not a resource conversion. Compare with an actual directed witness before drawing conclusions. [Forge Chain](../examples/forge-chain/README.md) demonstrates explicit pooling and disconnected controls. Standalone `view resources` is an incidence preview without unified route controls.
