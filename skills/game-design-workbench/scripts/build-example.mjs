#!/usr/bin/env node
// Maintainer tool: regenerate distribution templates and create a fresh tutorial.
// Usage: node scripts/build-example.mjs --templates --output <nonexistent-or-empty-dir>
// --templates deliberately rewrites only shipped generated template files.
import { mkdir, readdir, readFile, writeFile } from 'node:fs/promises';
import { resolve, dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createHash } from 'node:crypto';
import { model, schemaFor, usageFor } from './model.mjs';
import { records, fixtureIdentities } from './fixture-data.mjs';
const home = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const args = process.argv.slice(2);
if (args.includes('--help')) {
  console.log('build-example.mjs [--templates] --output <empty-directory>');
  process.exit(0);
}
const outputIndex = args.indexOf('--output');
if (outputIndex < 0 || !args[outputIndex + 1] || args.some((a, i) => !['--templates', '--output'].includes(a) && i !== outputIndex + 1)) throw new Error('Usage: build-example.mjs [--templates] --output <empty-directory>');
const output = resolve(args[outputIndex + 1]);
let existing = [];
try { existing = await readdir(output); } catch (error) { if (error.code !== 'ENOENT') throw error; }
if (existing.length) throw new Error('Output must be empty. Refusing to overwrite tutorial edits.');
const frontmatter = data => `---\n${JSON.stringify(data, null, 2)}\n---\n`;
const put = async (path, text) => { await mkdir(dirname(path), { recursive: true }); await writeFile(path, text, { flag: 'wx' }); };
await put(join(output, 'game-design/concept.md'), (await readFile(join(home, 'templates/concept.md'), 'utf8')).replace('{{mode}}', 'game-explorer'));
for (const role of Object.keys(model)) {
  const files = { '.schema.md': `${frontmatter(schemaFor(role))}# ${role} schema\n\nGenerated from scripts/model.mjs for distribution. Project-local schema remains authoritative.\n`, '.usage.md': usageFor(role), '.template.md': model[role].sections.map(section => `## ${section}\n`).join('\n') };
  for (const [name, content] of Object.entries(files)) {
    if (args.includes('--templates')) {
      const path = join(home, 'templates', role, name);
      await mkdir(dirname(path), { recursive: true });
      await writeFile(path, content);
    }
    await put(join(output, 'game-design', role, name), content);
  }
}
await put(join(output, '.pi', 'attendant.tables'), Object.keys(model).map(role => `game-design/${role} as ${role}\n`).join(''));
await put(join(output, '.gitignore'), '.attendant/\ngame-design-results/*/artifacts/\n');
const created = '2026-10-19T00:00:00.000Z';
const timestamp = Date.parse(created).toString(16).padStart(12, '0');
for (const record of records) {
  const digest = createHash('sha256').update(`${record.role}/${record.name}`).digest('hex');
  const id = fixtureIdentities[`${record.role}/${record.name}`] ?? `${timestamp.slice(0, 8)}-${timestamp.slice(8)}-7${digest.slice(0, 3)}-8${digest.slice(3, 6)}-${digest.slice(6, 18)}`;
  const fields = {};
  for (const [key, declaration] of Object.entries(schemaFor(record.role))) fields[key] = Array.isArray(declaration) ? declaration[0] : declaration === 'string[]' || String(declaration).startsWith('?ref[]:') ? [] : String(declaration).startsWith('?ref:') || declaration === '?datetime' ? null : declaration;
  await put(join(output, 'game-design', record.role, `${record.name}.md`), `${frontmatter({ id, name: record.name, created_at: created, desc: record.desc, tags: [], ...fields, status: record.status, ...record.fields })}${record.body}\n`);
}
console.log(JSON.stringify({ ok: true, output, collections: Object.keys(model).length, records: records.length }));
