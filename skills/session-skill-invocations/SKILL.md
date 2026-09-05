---
name: session-skill-invocations
description: Report user and agent skill loads from saved Pi sessions. Use when auditing prior sessions for which skills were invoked, whether a user explicitly invoked a skill or the agent loaded its SKILL.md, and the associated session and JSONL line.
---

# Session Skill Invocations

Run the bundled report tool from this skill directory:

```bash
python scripts/report-session-skill-invocations.py
```

It writes a CSV table to stdout with:

- `conversation_hash`: Pi session UUID from the session header
- `timestamp`: ISO time of the invocation entry
- `jsonl_line`: one-indexed line number in the session JSONL file
- `skill_name`
- `loaded_by`: `user` or `agent`
- `session_file` and `skill_file` for traceability

Use JSONL when an automated consumer needs one record per line:

```bash
python scripts/report-session-skill-invocations.py --format jsonl
```

Use `--sessions-dir PATH` to select another session store. Use repeatable `--session-glob GLOB` to select files relative to that directory; `*` is the default and matches every saved session, and multiple globs are combined. Use repeatable `--skills-dir PATH` to match skills outside global Pi skill directories.

Use `--analysis-dir PATH` to write these additional CSV tables:

- `skill-usage-summary.csv`: loads, session reach, and loads per 100 conversation messages by skill and source.
- `skill-usage-over-time.csv`: daily loads and loads per 100 messages by skill and source.
- `skill-proximity.csv`: each consecutive skill-load pair in a session, with time and message distance.
- `skill-proximity-summary.csv`: median time and message distances grouped by consecutive skill/source pair.
- `skill-review-candidates.csv`: skills with zero or at most three detected loads, plus distinct ordered skill pairs used at least five times within five messages and one hour, where the pair covers at least 50% of the less-used skill's loads. These are review candidates, not automatic removal or merge decisions.

```bash
# All sessions (default)
python scripts/report-session-skill-invocations.py --session-glob '*'

# One conversation file, or a matching subset
python scripts/report-session-skill-invocations.py --session-glob '**/session.jsonl'
```

## Detection limits

Agent loads are detected from assistant `read` tool calls targeting `SKILL.md`. User loads are detected from persisted `/skill:<name>` messages, historical `<skill name="…">` wrappers, and older complete-skill message content. When an assistant immediately reads the skill named by a user command, the report attributes that load to the user instead of duplicating it as an agent load. A user who pasted identical skill content is indistinguishable from an explicit command.
