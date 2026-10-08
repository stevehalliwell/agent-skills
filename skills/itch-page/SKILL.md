---
name: itch-page
description: "Create an itch.io project page as Draft, set up its theme, or prepare a local itch page kit for a downloadable build, browser game, assets, tool, or physical game. Use for listing copy, delivery settings, media, and draft-page setup. Skip build-only uploads and public-release requests; publishing needs separate explicit approval."
---

# itch.io page kit

Prepare local copy and settings; create and theme a Draft only when live setup is requested.

## Trigger clarification

Identify kit-only or live setup from the request. Inspect project evidence first; ask one focused question only if scope remains unclear. Manual invocation starts that inspection, not a load acknowledgement.

- “Prepare an itch page kit”: write local files; no remote changes.
- “Set up our itch page”: prepare kit, then create Draft and theme it, with separate save approvals.
- “Push this build with butler”: outside this skill.

Done when scope is known before browser work.

## Required read

Read [page-template.md](references/page-template.md) when creating or updating the kit. It maps listing/theme fields, not a submission payload. Field limits are reference values; confirm current requirements in the live editor. Saved forms, if supplied, are examples, not defaults; never copy tokens or unrelated account details.

## Workflow

1. **Locate project and kit.** Inspect notes, builds, images, and prior kit. Default to `<project-root>/itch-page/page.md` or existing release/publishing folder. Preserve existing work; create `assets/` only for actual assets. Ask if project identity/root is unknown. Done when identity, output path, and existing evidence are known.
2. **Choose listing from actual delivery.** Establish project/build type, platforms, download/browser/both, and build availability before choosing form settings. Record title, account/slug, classification, itch kind, release status, pricing, and visibility intent. Ask material unknowns one at a time; record answers immediately. Default to Draft; never presume paid pricing or invent public URL. Check slug availability live. Done when choices are recorded or marked `Pending` with reasons.
3. **Draft evidence-backed copy.** Write distinct tagline, description, features, controls/usage, access needs, install instructions, genre, up to 10 relevant tags without duplicating genre/platforms, optional video/store links, custom noun, and community setting. Distinguish proposed copy from verified facts; resolve unsupported claims. Keep rich-text copy paste-ready; Markdown is not an itch editor import format. Done when applicable fields have usable copy or explicit gaps.
4. **Check delivery and media.** Inventory files, architecture, packaging, requirements, and optional upload channels.
   - Downloads: record files, launch/install steps, prerequisites; omit embed settings.
   - HTML: verify `index.html` at ZIP root or self-contained `.html`; record embed mode, viewport, tested mobile support, autoplay, fullscreen, scrollbars, and SharedArrayBuffer needs.
   - Physical games/assets/tools: record formats and usage.
   - Capture/request 3–5 real gameplay screenshots when applicable, otherwise accurate previews. Never label mockups or AI art as gameplay. Create/request cover only with authorized assets/tools; record source, rights, dimensions, approval, and paths. Cover reference: minimum 315×250 px, recommended 630×500 px; PNG/GIF/JPG/JPEG up to 3 MiB. Recheck live limits.
   Done when files/media are verified or missing items are explicit.
5. **Resolve disclosure and theme.** Ask whether project contains generative-AI output, including edited output; record answer and disclosure selection without guessing. Check pricing/payment setup, links, platform tags, readiness, and quality-guideline issues. Plan colors, opacity, fonts/sizes, screenshot placement, banner/alignment, and background using template. Use approved art or ask visual direction; check contrast and legibility. Keep cover separate from theme images; record rights/crop notes. Done when every applicable field has value, `Pending`, or `Use default`, plus remaining actions.
6. **Review kit.** Check against project evidence and template: no placeholders in paste-ready copy, unsupported claims, unrelated branches, or missing required fields. Correct and recheck. Required unknowns block affected remote actions, not local kit completion; record blocker rather than inventing values. Kit-only skips steps 7–8 and proceeds to report. Done when kit is usable or blockers are explicit.
7. **Create Draft, if requested.** Load `agent-browser`; use live new-project form. Compare controls with kit, resolve required gaps, verify rich-text formatting and cover crop, select **Draft**. Get approval before clicking Save & view page. Verify resulting URL and Draft state; record both. On failure or unexpected visibility, stop remote work and record blocker. Done when Draft exists and is verified.
8. **Theme verified Draft.** Open Edit theme; apply agreed settings/assets, preview desktop/mobile, check contrast, cropping, and readability. Correct and recheck; get approval before saving. Verify saved theme and Draft state. Record planned versus applied values and gaps. If checks fail, stop and record blocker, not success. Done when theme persists on Draft or blocker is recorded.
9. **Report state.** Give kit path, Draft URL/verified state if created, theme status, and remaining blockers. Never claim remote actions for kit-only work. Done when local and remote results are distinct.

## Boundaries

- Never enable Public or Restricted without explicit approval. Publishing is outside this workflow.
- Do not log in or upload builds without user direction and required confirmation. Approved listing/theme saves cover agreed media; build upload is separate.
- Butler uploads to an existing page; it cannot create/manage listing or theme. Optional later command: `butler push <build-folder> <account>/<slug>:<channel>`. Verify build/destination/channel and obtain upload approval; do not run during kit preparation.
- If image tools are unavailable, request files or record gaps; never claim capture/generation occurred. Verify third-party media rights.
