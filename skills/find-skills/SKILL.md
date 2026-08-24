---
name: find-skills
description: "Use when users ask for a skill to do something, say 'find a skill for this' or 'is there a skill that can do this', ask how to do something that may have an installable skill, or write an unrecognized /<word> command in a sentence. Discover relevant skills, assess fit, present options, and install only with explicit approval."
---

# Find Skills

Find installable skills for a requested capability.

## Workflow

1. Clarify the capability when needed: goal, important constraints, and whether the user wants discovery or installation.
   - Done when search terms and desired outcome are clear.
2. Search the skills registry:

   ```bash
   npx skills find "<focused query>"
   ```

   Use a few focused terms when the first search is too broad. Do not assume a skill exists.
   - Done when relevant candidates, or no candidates, are identified.
3. Read each promising candidate's skills page. If that is insufficient, read its public `SKILL.md` from the listed source. Summarize what it claims to do, or what its workflow actually does. Do not require GitHub CLI; ordinary public page/file access is enough. Omit candidates whose details cannot be retrieved.
   - Done when usable candidates have evidence-based summaries.
4. Present a short list with each candidate's name, summary, source, notable requirements or risks, and install command. Ask user to select from structured options; do not ask for a free-text choice when structured options fit.
   - Done when the user selects a candidate or declines.
5. Before installing the selected candidate, ask user where it belongs:
   - Current project folder: `npx skills add <package> -y`
   - Global agents skills folder: `npx skills add <package> -g -y`

   Confirm both destination and installation approval.
   - Done when destination and approval are explicit.
6. Run the selected install command. Report what was installed and any follow-up needed.
   - Done when installation succeeds, or its exact error is reported.

## Rules

- Prefer the smallest skill that solves the stated need.
- Do not recommend based only on popularity, install counts, vendor name, or search ranking.
- Do not install, update, or remove skills without explicit user approval.
- If no suitable skill is found, say so and offer to help directly. Suggest creating a skill only when the capability is recurring.
