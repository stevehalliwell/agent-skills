# Flow during visuomotor learning — research notes

## Source and rights

- Original title: *Flow Experiences During Visuomotor Skill Acquisition Reflect Deviation From a Power-Law Learning Curve, but Not Overall Level of Skill*.
- Authors: Benjamin Ultan Cowley, Jussi Palomäki, Tuisku Tammi, Roosa Frantsi, Ville-Pekka Inkilä, Noora Lehtonen, Pasi Pölönen, Juha Vepsäläinen, Otto Lappi.
- Published: 15 May 2019, *Frontiers in Psychology* 10:1126.
- DOI/original URL: https://doi.org/10.3389/fpsyg.2019.01126
- Full HTML: https://www.frontiersin.org/journals/psychology/articles/10.3389/fpsyg.2019.01126/full
- Publisher PDF: https://public-pages-files-2025.frontiersin.org/journals/psychology/articles/10.3389/fpsyg.2019.01126/pdf
- Access date: 2026-10-21.
- Source type: peer-reviewed original longitudinal experiment; later empirical work, not Csikszentmihalyi's original formulation and not GameFlow.
- Actual method for these notes: static HTML retrieved with `fetch_content`; abstract, introduction, protocol, methods, results, discussion, limitations, and copyright passages inspected with `get_search_content`. Independent notes, not converted article text. PDF conversion details recorded separately in README.
- Reuse basis: original publication explicitly licenses article CC BY 4.0, https://creativecommons.org/licenses/by/4.0/. Copyright © 2019 Cowley, Palomäki, Tammi, Frantsi, Inkilä, Lehtonen, Pölönen, Vepsäläinen and Lappi. Cite authors and original journal when reusing. Notes reorganize and summarize the source; article and figure extraction are format changes, not author-endorsed adaptations.
- [Actual local full article capture](cowley-2019-docling/cowley-2019-visuomotor-flow.md), reused 2026-10-21; notes are not that capture.
- Coverage limits: main article only. Supplementary files, experimental data, code, and physiological recordings not acquired. No independent replication or recalculation.

## Study design — §§1.1, 2

Nine adult convenience-sample participants learned a custom steering task across eight sessions and forty trials over 2–3 weeks. They steered around obstacles; speed rose automatically and collisions reduced speed. The task therefore changed its demands with performance rather than assigning one fixed difficulty to everyone. Goals, collision signals, completion time, and score feedback made progress explicit.

Participants completed the Flow Short Scale after each trial. The reported flow measure averaged ten items covering fluent performance and absorption. Three perceived-importance items were analyzed separately. Skill/demand-fit questions were collected at session end, not continuously. Physiological signals were recorded in some sessions but are outside this article's reported analysis.

This matters because performance, experience reports, and challenge assumptions are separate measurements. A designer's difficulty label is not equivalent to any of them.

## Findings — §3

- Performance improved with practice. Participant-specific power-law learning models described that improvement.
- Mean self-reported flow stayed elevated rather than rising reliably with accumulated skill. Group mean reported in discussion is 5.1 out of 7.
- Trial-by-trial flow covaried with performance relative to each participant's fitted learning curve: better-than-predicted times accompanied higher reports, and worse-than-predicted times accompanied lower reports.
- The mixed-model deviation coefficient was −8, with reported p = 0.002. Conditional pseudo-R² was 0.47. Conditional model fit includes participant effects; do not rephrase this as deviation scores alone causing or explaining 47% of flow.
- Perceived importance might moderate the relationship, but its group-level interaction was not significant. Treat this as unresolved, not a finding that task value determines flow.

## What this adds to the framework — §§1, 4

The article separates antecedent task conditions from experienced features such as focused involvement, reduced reflective self-consciousness, and altered time awareness. It also compares versions of flow theory rather than presenting one challenge/skill diagram as settled measurement.

Its theoretical issue is the reference level: if both challenge and skill must be above average, average relative to what, and does that reference move during learning? Stable reported flow alongside improving performance challenges a fixed-reference interpretation in this task. It does not settle every version of flow theory.

Repeated observations within a learning sequence can reveal relationships hidden by an overall success score. This is a reason to keep familiarity, trial history, and subjective accounts visible when investigating an encounter. It is not an instruction to fit a learning curve to arbitrary authored ranks.

## Limits and competing explanations — §§4.1.1, 4.2

- Nine participants provide extensive within-person data but limited population coverage. Repeated trials do not turn nine people into hundreds of independent participants.
- This is one deliberately flow-oriented visuomotor task. Transfer to puzzles, narrative play, ambiguous goals, or deliberate interruption is untested here.
- Association does not determine causal direction. Flow might aid performance; random obstacle arrangements might alter both performance and experience; seeing a successful score before answering might influence reports. Authors prefer one account but explicitly cannot conclusively select it.
- No independently manipulated set of clear-goal, feedback, and skill-match conditions establishes that any one feature caused the elevated mean.
- Self-reported flow is not interchangeable with directly observed absorption. The physiological data collected here do not provide a validated detector in this article.
- Does not establish flow, anxiety, or boredom in this repository's example game. Supplies research context and measurement cautions only.

## Complementarity

[Csikszentmihalyi's 2004 talk notes](csikszentmihalyi-2004-ted-source-notes.md) supply the originator's explanation and person-relative experience-sampling model. This experiment supplies later evidence and explicit disagreement about how reference levels should work during learning. Preserve that difference rather than silently merging the accounts.
