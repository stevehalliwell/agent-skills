#!/usr/bin/env node
// Maintainer generator. Edit pack-data.mjs, then --write; default --check never writes.
// Usage: node scripts/build-packs.mjs [--check|--write|--help]
import { mkdir, readFile, writeFile } from 'node:fs/promises';
import { join } from 'node:path';
import { packs } from './pack-data.mjs';
import { home } from './lib.mjs';
const args = process.argv.slice(2);
if (args.length > 1 || (args.length && !['--check', '--write', '--help'].includes(args[0]))) { console.error('Use --check (default), --write, or --help.'); process.exit(2); }
if (args[0] === '--help') { console.log('Maintainer: check or regenerate 18 original attributed packs/catalog from scripts/pack-data.mjs. --write replaces generated pack files; do not use on installed local amendments.'); process.exit(0); }
const write = args[0] === '--write';
const directory = join(home, 'references/frameworks');
const outputs = [];
for (const pack of packs) {
  const body = `# ${pack.title}\n\nPack: \`${pack.id}\` · version 1\nAttribution: ${pack.attribution}\n\n## Scope\n\n${pack.scope}\n\nUse only when this lens is selected. Follow [review operation](../review.md); do not repair source during review.\n\n## Inputs and query support\n\nRoles: ${pack.roles.map(role => `\`${role}\``).join(', ')}. Read actual role mapping and live schema/usage before writing.\n\nAvailable recipes: ${pack.queries.map(query => `\`query ${query}${query === 'records' ? ' --collection <mapped-alias>' : ''}\``).join('; ')}. These are declared-data projections, not automated framework verdicts. Inspect linked bodies for conditions and evidence.\n\n${pack.vocabulary ? `Vocabulary: ${pack.vocabulary}\n\n` : ''}## Original prompts\n\n${pack.questions.map((question, index) => `${index + 1}. ${question}`).join('\n')}\n\n## Missing data and evidence limits\n\n${pack.missing}\n\n${pack.limits}\n\n## Fixture application\n\n${pack.fixture}\n\n## Sources\n\n${pack.sources.map(([title, url]) => `- [${title}](${url})`).join('\n')}\n\nAccess checked 2026-10-19: ${pack.access}\n\nGuidance is independently authored. No book, proprietary deck, questionnaire, or scoring method is reproduced.\n`;
  outputs.push([`${pack.id}.md`, body]);
}
outputs.push(['catalog.json', `${JSON.stringify(packs.map(pack => ({ id: pack.id, title: pack.title, version: 1, path: `references/frameworks/${pack.id}.md`, attribution: pack.attribution, required_roles: pack.roles, queries: pack.queries })), null, 2)}\n`]);
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
