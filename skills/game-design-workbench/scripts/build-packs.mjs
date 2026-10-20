#!/usr/bin/env node
// Maintainer generator. Edit pack-data.mjs / pack-guidance.mjs, then --write.
// Default --check never writes. Usage: node scripts/build-packs.mjs [--check|--write|--help]
import { mkdir, readFile, writeFile } from 'node:fs/promises';
import { join } from 'node:path';
import { packs } from './pack-data.mjs';
import { guidance } from './pack-guidance.mjs';
import { home } from './lib.mjs';
const args = process.argv.slice(2);
if (args.length > 1 || (args.length && !['--check', '--write', '--help'].includes(args[0]))) { console.error('Use --check (default), --write, or --help.'); process.exit(2); }
if (args[0] === '--help') { console.log('Maintainer: check or regenerate 18 original attributed packs/catalog from scripts/pack-data.mjs and scripts/pack-guidance.mjs. --write replaces generated pack files; do not use on installed local amendments.'); process.exit(0); }
const write = args[0] === '--write';
const directory = join(home, 'references/frameworks');
const outputs = [];
const ids = new Set(packs.map(pack => pack.id));
if (ids.size !== packs.length || Object.keys(guidance).length !== ids.size || Object.keys(guidance).some(id => !ids.has(id))) throw Error('Pack guidance must match catalog IDs exactly.');
const bullets = values => values.map(value => `- ${value}`).join('\n');
const pairs = values => bullets(values.map(([label, text]) => `**${label}:** ${text}`));
const ordered = values => values.map((value, index) => `${index + 1}. ${value}`).join('\n');
for (const pack of packs) {
  const detail = guidance[pack.id];
  for (const key of ['concepts', 'procedure', 'example', 'evidence', 'traps']) {
    if (!Array.isArray(detail[key]) || !detail[key].length || detail[key].some(value => ['concepts', 'example'].includes(key) ? !Array.isArray(value) || value.length !== 2 || value.some(text => typeof text !== 'string' || !text.trim()) : typeof value !== 'string' || !value.trim())) throw Error(`Invalid ${key} guidance for ${pack.id}.`);
  }
  const sections = [
    ['Scope', pack.scope],
    ['Inputs and query support', `Roles: ${pack.roles.map(role => `\`${role}\``).join(', ')}.\n\nAvailable recipes: ${pack.queries.map(query => `\`query ${query}${query === 'records' ? ' --collection <mapped-alias>' : ''}\``).join('; ')}.`],
    ['Working concepts', `${pack.vocabulary ? `Vocabulary: ${pack.vocabulary}\n\n` : ''}${pairs(detail.concepts)}`],
    ['Apply to selected slice', ordered(detail.procedure)],
    ['Original prompts', ordered(pack.questions)],
    ['Worked example', pairs(detail.example)],
    ['Evidence and counterchecks', bullets(detail.evidence)],
    ['Common traps', bullets(detail.traps)],
    ['Missing data and evidence limits', `${pack.missing}\n\n${pack.limits}`],
    ['Sources', `${pack.sources.map(([title, url]) => `- [${title}](${url})`).join('\n')}\n\nAccess checked 2026-10-19: ${pack.access}`]
  ];
  const contents = sections.map(([title]) => `- [${title}](#${title.toLowerCase().replaceAll(' ', '-')})`).join('\n');
  const body = `# ${pack.title}\n\nPack: \`${pack.id}\` · version 1\nAttribution: ${pack.attribution}\n\n## Contents\n\n${contents}\n\n${sections.map(([title, text]) => `## ${title}\n\n${text}`).join('\n\n')}\n`;
  outputs.push([`${pack.id}.md`, body]);
}
outputs.push(['catalog.json', `${JSON.stringify(packs.map(pack => ({ id: pack.id, title: pack.title, version: 1, status: 'provisional', path: `references/frameworks/${pack.id}.md`, attribution: pack.attribution, required_roles: pack.roles, queries: pack.queries })), null, 2)}\n`]);
if (write) await mkdir(directory, { recursive: true });
const stale = [];
for (const [name, body] of outputs) {
  if (write) await writeFile(join(directory, name), body);
  else {
    const actual = await readFile(join(directory, name), 'utf8').catch(error => { if (error.code === 'ENOENT') return null; throw error; });
    if (actual?.replaceAll('\r\n', '\n') !== body) stale.push(name);
  }
}
console.log(JSON.stringify({ ok: !stale.length, operation: write ? 'write' : 'check', packs: packs.length, stale }));
process.exitCode = stale.length ? 1 : 0;
