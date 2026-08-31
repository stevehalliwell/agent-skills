# Agent Skills

Put [Agent Skills](https://agentskills.io/) to work in Pi and compatible coding agents with `npx skills`. This collection guides project and task workflows, research and web crawling, writing and messaging, knowledge and style management, technical and design reviews, and local document, transcript, and tooling work.

## Install

```sh
npx skills add stevehalliwell/agent-skills --all
```

You can install an individual skill with `npx skills add stevehalliwell/agent-skills --skill <skill-name>`, but these skills may reference each other and are intended to be available together.

## Skill index

### Tools and delegation

- [`attendant`](skills/attendant/) — cli tools to generate sqlite dbs from schema light md collections. E.g. keeping a backlog of tasks in the repository.
- [`crawl4ai`](skills/crawl4ai/) — Crawls public pages with Dockerized Crawl4AI when ordinary fetches fall short.
- [`delegate-tasks`](skills/delegate-tasks/) — Launches bounded subagent or Codex jobs, then tracks their lifecycle, status, or cancellation.
- [`docker-local`](skills/docker-local/) — Guides safe local Docker workflows for skills that depend on Docker.
- [`docling-local`](skills/docling-local/) — Converts local documents and images to Markdown with Dockerized Docling.
- [`youtube-transcript-download`](skills/youtube-transcript-download/) — Retrieves YouTube transcripts and captions for later use.

### Project and task workflow

- [`add-todo`](skills/add-todo/) — Captures project-local tasks without inventing implementation details.
- [`backlog-capture`](skills/backlog-capture/) — Records ideas and deferred work for later refinement.
- [`backlog-refinement`](skills/backlog-refinement/) — Turns one unready backlog item into implementation-ready work at a time.
- [`coding`](skills/coding/) — Applies mandatory engineering judgment to every code change. Close to caveman and Ponytail.
- [`implementation`](skills/implementation/) — Delivers an agreed, reviewable implementation slice and validates it proportionately.
- [`init-project`](skills/init-project/) — Sets up project guidance, status documents, and Attendant records.
- [`iteration`](skills/iteration/) — Drives rapid, low-risk tweak loops without workflow churn.
- [`pick-up`](skills/pick-up/) — Rebuilds project context from repository state, handoffs, and tasks.
- [`release-readiness`](skills/release-readiness/) — Aligns release-facing documentation with changes since the last release.
- [`task-lifecycle`](skills/task-lifecycle/) — Governs status transitions and record rules for tracked Attendant tasks, including automatic record updates.
- [`task-refinement`](skills/task-refinement/) — Turns clear goals into defined behavior, boundaries, and acceptance criteria.
- [`wrap-up`](skills/wrap-up/) — Records progress and leaves concise context for the next session.

### Research, analysis, and decisions

- [`brainstorming`](skills/brainstorming/) — Expands and refines ideas before choosing a direction.
- [`competitor-analysis`](skills/competitor-analysis/) — Compares competitor positioning and offerings with evidence.
- [`five-whys`](skills/five-whys/) — Leads evidence-aware, non-blaming root-cause analysis.
- [`question-research`](skills/question-research/) — Builds retained, evidence-linked answers to focused questions.
- [`persona`](skills/persona/) — Runs focused, explicit role-based collaboration sessions.
- [`six-thinking-hats`](skills/six-thinking-hats/) — Separates facts, feelings, benefits, risks, and alternatives before decisions.
- [`technical-review`](skills/technical-review/) — Examines feasibility, architecture, delivery risk, and technical trade-offs before coding.
- [`tradeoff-review`](skills/tradeoff-review/) — Frames material cross-system choices and records agreed decisions.
- [`storybrand`](skills/storybrand/) — Builds or reviews BrandScripts and content through the StoryBrand framework.

### Writing, messaging, and content

- [`editorial-review`](skills/editorial-review/) — Sharpens supplied long-form drafts while preserving author intent.
- [`messaging-strategy`](skills/messaging-strategy/) — Strengthens customer-facing positioning, copy, and calls to action.
- [`note-taking`](skills/note-taking/) — Captures concise structured notes without creating durable project records.
- [`style-profile`](skills/style-profile/) — Learns reusable profiles from examples, writes or rewrites Markdown with them, and checks documents or corpora against them.
- [`transcript-to-prose`](skills/transcript-to-prose/) — Turns timestamped spoken text into readable paragraphs without rewriting it.

### Skill and knowledge management

- [`find-skills`](skills/find-skills/) — Finds suitable installable skills and installs them only with approval.
- [`okf`](skills/okf/) — Keeps Open Knowledge Format bundles conformant with the OKF specification.
- [`skill-craft`](skills/skill-craft/) — Builds, reviews, and improves predictable, maintainable agent skills.

### Web delivery

- [`threejs-performance`](skills/threejs-performance/) — Keeps Three.js work within agreed rendering budgets.
- [`web-implementation`](skills/web-implementation/) — Brings standards-aware constraints to web implementation and review.
- [`website-art-direction`](skills/website-art-direction/) — Shapes evidence-led visual directions for website design.

## Inspirations

- [`skill-craft`](skills/skill-craft/), [`pick-up`](skills/pick-up/), and [`wrap-up`](skills/wrap-up/) were inspired by [mattpocock/skills](https://github.com/mattpocock/skills).
- [`find-skills`](skills/find-skills/) was inspired by [Agentic Skills find-skills](https://agenticskills.io/skills/find-skills).
- [`persona`](skills/persona/) was inspired by [pi-facets](https://github.com/stevehalliwell/pi-facets).
- [`attendant`](skills/attendant/) was inspired by [pi-attendant](https://github.com/stevehalliwell/pi-attendant).
- [`youtube-transcript-download`](skills/youtube-transcript-download/) was inspired by [pi-youtube-transcript](https://pi.dev/packages/pi-youtube-transcript?page=48).

## Repository notes

- [`skills/`](skills/) is the canonical source directory. Keep its structure intact.
- Generated dependencies, bytecode, test output, and local environment files are ignored.

## License

[MIT](LICENSE)
