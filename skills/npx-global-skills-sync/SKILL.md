---
name: npx-global-skills-sync
description: "Sync, reconcile, or audit globally installed npx skills against Pi's tracked skills_sync.json. Use when comparing npx global skills with Pi, checking skill junctions or symlinks, restoring global npx skills on another machine, or deciding whether to add, remove, install, or uninstall a globally managed skill."
---

# npx Global Skills Sync

Reconcile `npx skills` global installs with Pi's tracked manifest without touching personal Pi skills.

## Locations and ownership

- `npx skills` lock: `%USERPROFILE%\.agents\.skill-lock.json`
- `npx skills` canonical files: `%USERPROFILE%\.agents\skills\`
- Pi global skills: `%USERPROFILE%\.pi\agent\skills\`
- Tracked manifest: `%USERPROFILE%\.pi\agent\skills_sync.json`

`skills_sync.json` is the desired, portable record. `.skill-lock.json` is owned by `npx skills`; never edit it directly. Preserve skills in Pi's global folder that do not resolve into `.agents\skills`.

## Manifest format

```json
{
  "version": 1,
  "skills": {
    "skill-name": {
      "source": "owner/repository",
      "sourceType": "github",
      "sourceUrl": "https://github.com/owner/repository.git",
      "skillPath": "skills/skill-name/SKILL.md",
      "skillFolderHash": "upstream-or-content-hash"
    }
  }
}
```

Keep only source fields supplied by `.skill-lock.json`. Do not copy `installedAt`, `updatedAt`, or `dismissed`.

## Workflow

1. Locate the four paths. If the manifest does not exist, read the npx lock and create it with version `1` and every current npx lock entry. Do not modify npx installs during this initialization. Done when both records can be parsed.

2. Run the bundled checker before inspecting individual entries:

   ```bash
   node <skill-directory>/scripts/check-npx-global-skills-sync.mjs --json
   ```

   It resolves junctions and symlinks, including those reached through a junctioned skills directory. Treat only entries resolving under `%USERPROFILE%\.agents\skills` as npx-managed; preserve personal Pi skills. Done when checker output lists every managed name and resolved target.

3. Use checker output to compare the `skills` maps by name and these fields when present: `source`, `sourceType`, `sourceUrl`, `skillPath`, and `skillFolderHash`. Classify each difference:
   - **npx-only**: present in `.skill-lock.json` and linked into Pi, absent from `skills_sync.json`.
   - **manifest-only**: present in `skills_sync.json`, absent from the npx lock or absent from Pi's npx-managed links.
   - **metadata mismatch**: present in both with differing tracked fields.
   - **link mismatch**: present in both records but not linked from Pi's global skills directory to the matching npx skill.
   - **orphaned Pi link**: Pi link resolves under `.agents\skills`, but no matching npx lock entry exists.
   If no differences exist, say they are aligned and take no action. Done when each difference has one classification and its relevant metadata is shown.

4. Handle one difference at a time. State the name, classification, source metadata, and paths. Ask with `ask_user` what to do; do not batch confirmations. Use these explicit flows:
   - **npx-only**: offer **Add to global manifest** or **Remove from npx skills**.
   - **manifest-only**: offer **Install via npx skills** or **Remove from global manifest**.
   - **metadata mismatch**: offer **Accept npx state into global manifest** or **Reinstall npx from manifest source**.
   - **link mismatch**: offer **Restore global npx links** or **Remove from npx skills**.
   - **orphaned Pi link**: offer **Remove stale Pi link** or **Leave unresolved**. Do not invent source metadata for an entry absent from the npx lock.

   **Add to global manifest** copies the npx lock entry's tracked fields to `skills_sync.json`. **Remove from global manifest** deletes only that entry. **Install via npx skills**, **Reinstall npx from manifest source**, and **Restore global npx links** run `npx skills add <source> --skill <name> -g --agent '*' -y`; this adds the skill to every detected agent's global skills directory. Re-read the npx lock and update the manifest entry from its resulting tracked fields. **Remove from npx skills** runs `npx skills remove --global <name> -y`; this removes the skill from every npx-managed global agent, then deletes its manifest entry. **Remove stale Pi link** removes only the named link after confirming its resolved target is under `.agents\skills`.

   After every selected action, run the bundled checker. Confirm the expected classification is gone, and report any new remaining classification instead of declaring success. Done when the chosen action is complete and both records are re-read.

5. Continue through remaining differences. Validate `skills_sync.json` as JSON after every edit. At completion, report aligned names, unresolved differences, and changed files. Remind the user to commit `C:\Users\steve\.pi\agent\skills_sync.json`; never commit for them. Done when all differences are resolved or deliberately left unchanged.

## Rules

- Ask before every install, uninstall, manifest deletion, or link repair.
- Never remove or overwrite an unlinked personal Pi skill.
- Do not run `npx skills update` as part of reconciliation. Update only when the user explicitly asks.
- Use the bundled checker rather than reimplementing its comparison in an ad-hoc command.
- Use `npx skills` commands rather than editing its lock or canonical files.
- Use absolute Windows paths in user-facing findings.
