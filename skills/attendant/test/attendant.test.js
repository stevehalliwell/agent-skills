import assert from "node:assert/strict";
import { spawnSync } from "node:child_process";
import { createHash } from "node:crypto";
import { mkdtemp, mkdir, readFile, rm, writeFile } from "node:fs/promises";
import { tmpdir } from "node:os";
import { dirname, join } from "node:path";
import test from "node:test";
import { fileURLToPath } from "node:url";

const runner = fileURLToPath(new URL("../scripts/attendant.mjs", import.meta.url));

async function fixture(files) {
  const root = await mkdtemp(join(tmpdir(), "attendant-skill-"));
  for (const [path, content] of Object.entries(files)) {
    const target = join(root, path);
    await mkdir(dirname(target), { recursive: true });
    await writeFile(target, content);
  }
  return root;
}

function run(root, args, input) {
  const result = spawnSync(process.execPath, [runner, ...args], { cwd: root, input, encoding: "utf8" });
  return { ...result, json: result.stdout ? JSON.parse(result.stdout) : undefined };
}

function record(name, body = "") {
  return `---\nid: 0197f3f1-1a2b-7000-8000-0000000000${name.length.toString().padStart(2, "0")}\nname: ${name}\ncreated_at: 2025-01-01T00:00:00Z\ndesc: ${name}\ntags: [search]\nstatus: open\n---\n${body}`;
}

test("runner creates an empty collection from a temporary fixture", async (t) => {
  const root = await fixture({ ".pi/attendant.tables": "" });
  t.after(() => rm(root, { recursive: true, force: true }));

  const result = run(root, ["add-table", "-d", "records/notes", "-a", "notes"]);
  assert.equal(result.status, 0, result.stderr);
  assert.equal(result.json.name, "notes");
  assert.equal(await readFile(join(root, ".pi/attendant.tables"), "utf8"), "records/notes as notes\n");
  assert.equal(await readFile(join(root, "records/notes/.schema.md"), "utf8"), "---\n---\n");
  assert.equal(await readFile(join(root, "records/notes/.template.md"), "utf8"), "");
});

test("runner validates, projects, queries, searches, schemas, and creates records", async (t) => {
  const root = await fixture({
    ".pi/attendant.tables": "todos/\n",
    "todos/.schema.md": "---\nstatus: [open, done]\npriority: 0\n---\n",
    "todos/alpha.md": record("alpha", "searchable SQLite projection\n"),
  });
  t.after(() => rm(root, { recursive: true, force: true }));

  const validated = run(root, ["validate"]);
  assert.equal(validated.status, 0, validated.stderr);
  assert.equal(validated.json.diagnostics.length, 0);
  const synced = run(root, ["sync"]);
  assert.equal(synced.status, 0, synced.stderr);
  assert.equal(run(root, ["schema"]).json.collections[0].name, "todos");
  const queried = run(root, ["query", "-s", "SELECT name FROM todos WHERE name = :name", "-P", '{"name":"alpha"}']);
  assert.equal(queried.status, 0, queried.stderr);
  assert.deepEqual(queried.json.rows, [{ name: "alpha" }]);
  const searched = run(root, ["search", "-q", "SQLite"]);
  assert.equal(searched.status, 0, searched.stderr);
  assert.equal(searched.json.rows[0].name, "alpha");
  const created = run(root, ["create", "-c", "todos", "-i", '[{"name":"beta","fields":{"status":"done","priority":2,"desc":"Complete documentation review","tags":["docs"]}}]']);
  assert.equal(created.status, 0, created.stderr);
  assert.equal(created.json.records[0].fields.name, "beta");
  assert.match(await readFile(join(root, "todos/beta.md"), "utf8"), /status: done/);
  assert.match(await readFile(join(root, "todos/beta.md"), "utf8"), /desc: Complete documentation review/);
  assert.match(await readFile(join(root, "todos/beta.md"), "utf8"), /tags:\n  - docs/);

  const batch = run(root, ["create", "-c", "todos", "-i", '[{"name":"gamma","fields":{"priority":1}},{"name":"delta","fields":{"status":"done","desc":"Finish docs"}}]']);
  assert.equal(batch.status, 0, batch.stderr);
  assert.deepEqual(batch.json.records.map((item) => item.fields.name), ["gamma", "delta"]);
  assert.match(await readFile(join(root, "todos/gamma.md"), "utf8"), /priority: 1/);
  assert.match(await readFile(join(root, "todos/delta.md"), "utf8"), /status: done/);

  const invalidBatch = run(root, ["create", "-c", "todos", "-i", '[{"name":"epsilon","fields":{"missing":true}},{"name":"zeta"}]']);
  assert.notEqual(invalidBatch.status, 0);
  await assert.rejects(readFile(join(root, "todos/epsilon.md"), "utf8"));
});

test("runner updates one or many records only after batch validation", async (t) => {
  const root = await fixture({
    ".pi/attendant.tables": "todos/\n",
    "todos/.schema.md": "---\nstatus: [open, done]\n---\n",
    "todos/alpha.md": record("alpha", "alpha body\n"),
    "todos/beta.md": record("beta", "beta body\n"),
  });
  t.after(() => rm(root, { recursive: true, force: true }));

  const before = await readFile(join(root, "todos/alpha.md"), "utf8");
  const rejected = run(root, ["update", "-c", "todos", "-i", '[{"name":"alpha","fields":{"status":"done"}},{"name":"beta","fields":{"status":"invalid"}}]']);
  assert.notEqual(rejected.status, 0);
  assert.equal(await readFile(join(root, "todos/alpha.md"), "utf8"), before);

  const updated = run(root, ["update", "-c", "todos", "-i", '[{"name":"alpha","fields":{"status":"done"}},{"name":"beta","fields":{"status":"done"}}]']);
  assert.equal(updated.status, 0, updated.stderr);
  assert.equal(updated.json.records.length, 2);
  assert.match(await readFile(join(root, "todos/alpha.md"), "utf8"), /status: done/);
  assert.match(await readFile(join(root, "todos/alpha.md"), "utf8"), /alpha body/);
  const queried = run(root, ["query", "-s", "SELECT name FROM todos WHERE status = :status ORDER BY name", "-P", '{"status":"done"}']);
  assert.deepEqual(queried.json.rows, [{ name: "alpha" }, { name: "beta" }]);
  const obsoleteSingle = run(root, ["update", "-c", "todos", "-n", "alpha", "-f", '{"status":"open"}']);
  assert.notEqual(obsoleteSingle.status, 0);
  assert.match(obsoleteSingle.stderr, /not valid for update/);
});

test("runner rejects malformed transport and preserves strict validation source", async (t) => {
  const root = await fixture({
    ".pi/attendant.tables": "todos/\n",
    "todos/.schema.md": "---\nstatus: [open]\n---\n",
    "todos/item.md": "---\nname: item\n---\nbody\n",
  });
  t.after(() => rm(root, { recursive: true, force: true }));

  const invalidJson = run(root, ["create", "-c", "todos", "-i", "{bad"]);
  assert.notEqual(invalidJson.status, 0);
  assert.match(invalidJson.stderr, /cli-json: invalid JSON/);
  const strict = run(root, ["validate", "--strict"]);
  assert.equal(strict.status, 1);
  assert.equal(strict.json.corrected, false);
  assert.match(await readFile(join(root, "todos/item.md"), "utf8"), /name: item/);
});

test("runner accepts @file and stdin input and contains query writes", async (t) => {
  const root = await fixture({
    ".pi/attendant.tables": "todos/\n",
    "todos/.schema.md": "---\n---\n",
    "todos/item.md": record("item", "needle\n"),
  });
  t.after(() => rm(root, { recursive: true, force: true }));
  await writeFile(join(root, "query.sql"), "SELECT name FROM todos");
  await writeFile(join(root, "params.json"), '{"name":"item"}');

  const fromFile = run(root, ["query", "--sql", "@query.sql"]);
  assert.equal(fromFile.status, 0, fromFile.stderr);
  assert.equal(fromFile.json.rows[0].name, "item");
  const paramsFromFile = run(root, ["query", "--sql", "SELECT name FROM todos WHERE name = :name", "--params", "@params.json"]);
  assert.equal(paramsFromFile.status, 0, paramsFromFile.stderr);
  assert.equal(paramsFromFile.json.rows[0].name, "item");
  const stdin = run(root, ["search", "--query", "-"], "needle");
  assert.equal(stdin.status, 0, stdin.stderr);
  const write = run(root, ["query", "--sql", "DELETE FROM todos"]);
  assert.notEqual(write.status, 0);
  assert.match(write.stderr, /read-only|prohibited|denied/i);
});

test("runner checks and applies reviewed Markdown migration plans", async (t) => {
  const root = await fixture({ ".pi/attendant.tables": "", "inbox/idea.md": "# Idea\n\nUseful body\n" });
  t.after(() => rm(root, { recursive: true, force: true }));
  for (const args of [["init", "-q"], ["config", "user.email", "test@example.com"], ["config", "user.name", "Test"], ["add", "-A"], ["commit", "-qm", "baseline"]]) {
    assert.equal(spawnSync("git", args, { cwd: root }).status, 0);
  }
  const source = await readFile(join(root, "inbox/idea.md"), "utf8");
  const hash = createHash("sha256").update(source).digest("hex");
  await mkdir(join(root, "migrations"));
  await writeFile(join(root, "migrations/notes.md"), `---
status: ready
collections:
  - directory: records/notes
    schema: |
      ---
      ---
files:
  - source: inbox/idea.md
    destination: records/notes/idea.md
    source_hash: ${hash}
    fields: {title: Idea}
    remove:
      - start: 0
        end: 8
        text: "# Idea\\n\\n"
---
`);

  const checked = run(root, ["migrate", "check", "--plan", "migrations/notes.md"]);
  assert.equal(checked.status, 0, checked.stderr);
  assert.equal(checked.json.ok, true);
  const applied = run(root, ["migrate", "apply", "--plan", "migrations/notes.md"]);
  assert.equal(applied.status, 0, applied.stderr);
  assert.equal(applied.json.applied, true);
  assert.match(await readFile(join(root, "records/notes/idea.md"), "utf8"), /title: Idea/);
});
