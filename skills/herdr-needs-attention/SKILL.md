---
name: herdr-needs-attention
description: "Mark this Herdr pane as needing attention, still waiting on me, or awaiting review. Use when user requests an attention marker or work must pause for their decision, review, or external action. Keep a question prompt open until they respond. Skip ordinary completion with nothing pending."
---

# Herdr Needs Attention

Use an open question prompt as attention marker. Herdr's question integration reports `blocked`; viewing pane does not answer prompt.

1. Check `HERDR_ENV=1`. Outside Herdr, report that no Herdr marker can be set and stop.
2. State pending action briefly. Include relevant file paths or review result before opening prompt.
3. Immediately call `ask_user` with pending decision and real options. For review or external work, ask user to respond when ready; offer `Ready — resume` and `Stop here`. Custom text can supply feedback. Do not add another approval gate for work already authorized.
4. Leave prompt open while waiting. Do not poll, send notifications, or overwrite Herdr agent state through CLI.
5. On response, follow supplied direction. Readiness alone does not approve changes, commits, or release. Cancellation is not approval: leave work paused and report that attention marker was dismissed. Do not reopen automatically.

Example: “Mark yourself as still needing my review.” Present review paths, then open `ask_user`: “Review these changes when available. Respond when ready, or type feedback.”

Requires interactive question tool and working Herdr question integration. If unavailable, report limitation; do not claim pane is flagged. A skill cannot preserve marker after prompt is dismissed or session exits.
