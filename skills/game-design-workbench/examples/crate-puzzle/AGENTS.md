## Attendant

- Markdown is source of truth; `.attendant/` is disposable projection state.
- `.pi/attendant.tables` lists collection directories/aliases. Each configured directory owns `.schema.md` and `.usage.md`; read both before operating on its records.
- Load separately installed `attendant` skill and its matching operation workflow. Do not copy its code or edit generated projection files.

## Game design workbench

- Load installed `game-design-workbench` skill for game-design operations. In this repository, its source is `../../SKILL.md` relative to this example root.
- Live concept/mode: `game-design/concept.md`. Role aliases match directory basenames in registry; `skill-atoms` means game-design skill atoms, not agent skills.
- Use Attendant and ordinary Git directly. No workbench CLI, manifest, or local runtime installation.
- `game-design-results/` and `review-baseline*.json` contain historical evidence. No old report is a fresh review. Experiments remain proposed/unexecuted.
