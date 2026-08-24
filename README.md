# Agent Skills

A reusable collection of [Agent Skills](https://agentskills.io/) for Pi and compatible coding agents, designed to work with `npx skills`. It covers project and task workflows, research and web crawling, writing and messaging, knowledge and style management, technical and design reviews, and local document, transcript, and tooling workflows.

## Install

```sh
npx skills add stevehalliwell/agent-skills --all
```

You can install an individual skill with `npx skills add stevehalliwell/agent-skills --skill <skill-name>`, but these skills may reference each other and are intended to be available together.

## Skill index

### Tools and delegation

- [`attendant`](skills/attendant/) — Sets up, queries, validates, and migrates Attendant-managed Markdown records. Use it for collections, schemas, projections, and record operations rather than ordinary Markdown editing.
- [`crawl4ai`](skills/crawl4ai/) — Crawls public web pages with Crawl4AI when ordinary fetches are insufficient. It supports rendered pages, structured multi-page crawling, screenshots, MHTML, and retained source content within a bounded, permitted scope.
- [`delegate-tasks`](skills/delegate-tasks/) — Runs bounded subagent or Codex jobs and manages their lifecycle. It supports parallel investigation, second opinions, background jobs, and status or cancellation checks.
- [`docker-local`](skills/docker-local/) — Provides shared safety and lifecycle guidance for local Docker-backed skills.
- [`docling-local`](skills/docling-local/) — Converts local documents and images to Markdown with Dockerized Docling.
- [`youtube-transcript-download`](skills/youtube-transcript-download/) — Downloads a YouTube transcript or captions for later use.

### Project and task workflow

- [`add-todo`](skills/add-todo/) — Captures a project-local task without inventing its implementation details.
- [`backlog-capture`](skills/backlog-capture/) — Quickly records ideas and deferred work for later refinement.
- [`backlog-refinement`](skills/backlog-refinement/) — Turns one unready backlog item at a time into implementation-ready work.
- [`implementation`](skills/implementation/) — Delivers one agreed, reviewable implementation slice with proportionate validation.
- [`init-project`](skills/init-project/) — Bootstraps project guidance, status documents, and Attendant records.
- [`iteration`](skills/iteration/) — Handles rapid, low-risk tweak loops without workflow churn.
- [`pick-up`](skills/pick-up/) — Resumes a project from its repository state, handoff, and tasks.
- [`release-readiness`](skills/release-readiness/) — Reconciles release-facing documentation with changes since the last release.
- [`task-lifecycle`](skills/task-lifecycle/) — Defines status transitions and record rules for tracked Attendant tasks.
- [`task-refinement`](skills/task-refinement/) — Shapes a clear goal into behavior, boundaries, and acceptance criteria.
- [`wrap-up`](skills/wrap-up/) — Records progress and creates concise context for the next session.

### Research, analysis, and decisions

- [`brainstorming`](skills/brainstorming/) — Expands and refines ideas without prematurely selecting a direction.
- [`competitor-analysis`](skills/competitor-analysis/) — Produces evidence-backed comparisons of competitor positioning and offerings.
- [`five-whys`](skills/five-whys/) — Guides an evidence-aware, non-blaming root-cause analysis.
- [`question-research`](skills/question-research/) — Builds retained, evidence-linked answers to one focused question.
- [`six-thinking-hats`](skills/six-thinking-hats/) — Separates facts, feelings, benefits, risks, and alternatives before a decision.
- [`technical-review`](skills/technical-review/) — Assesses feasibility, architecture, delivery risk, and technical trade-offs before coding.
- [`tradeoff-review`](skills/tradeoff-review/) — Frames material cross-system choices and records an agreed decision.
- [`storybrand`](skills/storybrand/) — Creates or reviews BrandScripts and content through the StoryBrand framework.

### Writing, messaging, and content

- [`editorial-review`](skills/editorial-review/) — Revises a supplied long-form draft while preserving author intent.
- [`messaging-strategy`](skills/messaging-strategy/) — Reviews and improves customer-facing positioning, copy, and calls to action.
- [`note-taking`](skills/note-taking/) — Captures concise structured notes without creating durable project records.
- [`style-profile`](skills/style-profile/) — Learns reusable profiles from examples, generates or rewrites Markdown with a profile, and verifies a document or corpus against one. It routes to the appropriate workflow for the requested outcome.
- [`transcript-to-prose`](skills/transcript-to-prose/) — Cleans timestamped spoken text into readable paragraphs without rewriting it.

### Skill and knowledge management

- [`find-skills`](skills/find-skills/) — Finds suitable installable skills and installs them only with approval.
- [`okf`](skills/okf/) — Maintains Open Knowledge Format bundles and their conformance to the OKF specification.
- [`skill-craft`](skills/skill-craft/) — Creates, reviews, and improves predictable, maintainable agent skills.

### Web delivery

- [`threejs-performance`](skills/threejs-performance/) — Protects rendering budgets during Three.js performance work.
- [`web-implementation`](skills/web-implementation/) — Applies standards-aware constraints to web implementation and review.
- [`website-art-direction`](skills/website-art-direction/) — Develops evidence-led visual directions for website design.

## Repository notes

- [`skills/`](skills/) is the canonical source directory. Keep its structure intact.
- Generated dependencies, bytecode, test output, and local environment files are ignored.

## License

[MIT](LICENSE)
