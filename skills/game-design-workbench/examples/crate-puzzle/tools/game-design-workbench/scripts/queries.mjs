// Read-only Attendant SQL recipes. No prerequisite/rank/economy analysis engine.
import { mkdtemp, mkdir, writeFile, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join, dirname, relative } from 'node:path';
import { collections, attendant, snapshot, optional, safePath, fail, slash } from './lib.mjs';
const quote = name => `"${name.replaceAll('"', '""')}"`;
export const queries = {
  matrix: { needs: { interactions: ['status', 'nouns', 'verb', 'coverage'] }, sql: m => `SELECT nouns, verb, '${m.interactions}/' || name AS interaction, coverage, desc, source_path FROM ${quote(m.interactions)} WHERE status = 'accepted' ORDER BY name` },
  'skill-atoms': { needs: { 'skill-atoms': ['status', 'rank', 'prerequisites', 'mechanics'] }, sql: m => `SELECT name, desc, rank, prerequisites, mechanics, source_path FROM ${quote(m['skill-atoms'])} WHERE status = 'accepted' ORDER BY name` },
  routes: { needs: { challenges: ['status', 'entry', 'next', 'introduces', 'practises', 'requires', 'tests_mastery'] }, sql: m => `SELECT name, entry, next, introduces, practises, requires, tests_mastery, source_path FROM ${quote(m.challenges)} WHERE status = 'accepted' ORDER BY name` },
  mda: { needs: { aesthetics: ['status', 'dynamics'], dynamics: ['status', 'mechanics', 'systems'], systems: ['status', 'mechanics'], mechanics: ['status'] }, sql: m => `SELECT 'aesthetics' AS role, name, 'dynamics' AS link_kind, dynamics AS links, source_path FROM ${quote(m.aesthetics)} WHERE status='accepted' UNION ALL SELECT 'dynamics', name, 'mechanics', mechanics, source_path FROM ${quote(m.dynamics)} WHERE status='accepted' UNION ALL SELECT 'dynamics', name, 'systems', systems, source_path FROM ${quote(m.dynamics)} WHERE status='accepted' UNION ALL SELECT 'systems', name, 'mechanics', mechanics, source_path FROM ${quote(m.systems)} WHERE status='accepted' UNION ALL SELECT 'mechanics', name, NULL, NULL, source_path FROM ${quote(m.mechanics)} WHERE status='accepted' ORDER BY role, name, link_kind` },
  decisions: { needs: { decisions: ['status', 'record_kind', 'alternative_group', 'subjects'] }, sql: m => `SELECT name, desc, status, record_kind, alternative_group, subjects, source_path FROM ${quote(m.decisions)} ORDER BY name` },
  glossary: { needs: { glossary: ['term', 'aliases', 'documentation', 'status'] }, sql: m => `SELECT name, term, aliases, documentation, status, source_path FROM ${quote(m.glossary)} ORDER BY name` },
  controls: { needs: { controls: ['status', 'value_type', 'value', 'expression', 'unit'] }, sql: m => `SELECT name, value_type, value, expression, unit, source_path FROM ${quote(m.controls)} WHERE status='accepted' ORDER BY name` },
  evidence: { needs: { experiments: ['status', 'stage', 'experiment_kind'] }, sql: m => `SELECT name, desc, status, stage, experiment_kind, source_path FROM ${quote(m.experiments)} ORDER BY name` }
};
export async function projected(root, operation) {
  const before = await snapshot(root);
  const configured = await collections(root);
  const temporary = await mkdtemp(join(tmpdir(), 'gd-query-'));
  try {
    for (const file of before.files) {
      const path = join(temporary, file.path); await mkdir(dirname(path), { recursive: true }); await writeFile(path, file.bytes);
    }
    // Absolute in-project source directories must point into temporary copy.
    await mkdir(join(temporary, '.pi'), { recursive: true });
    await writeFile(join(temporary, '.pi/attendant.tables'), configured.map(c => `${c.directory} as ${c.alias}\n`).join(''));
    const validation = attendant(temporary, ['validate', '--no-correct']);
    const rebase = value => {
      if (Array.isArray(value)) return value.map(rebase);
      if (value && typeof value === 'object') return Object.fromEntries(Object.entries(value).map(([key, item]) => [key, ['path', 'source_path', 'directory', 'schemaPath'].includes(key) && typeof item === 'string' && item.startsWith(temporary) ? slash(relative(temporary, item)) : rebase(item)]));
      return value;
    };
    const context = { temporary, schema: attendant(temporary, ['schema']), validation: rebase(validation), rebase, before };
    const data = await operation(context);
    if ((await snapshot(root)).metadata.fingerprint !== before.metadata.fingerprint) fail('SOURCE_CHANGED', 'Source changed during query. Retry against current state.');
    return { data: rebase(data), snapshot: before.metadata, projection_corrections: rebase(validation.corrections ?? []) };
  } finally { await rm(temporary, { recursive: true, force: true }); }
}
export async function runQuery(root, name, options = {}) {
  return projected(root, async context => {
    if (context.before.metadata.project_context?.status === 'invalid') fail(context.before.metadata.project_context.issue.code, context.before.metadata.project_context.issue.message);
    if (context.validation.diagnostics.length) fail('SOURCE_INVALID', 'Fix supplied invalid data before querying; validate reports source diagnostics.', context.validation);
    const bytes = await optional(await safePath(root, 'tools/game-design-workbench/manifest.json'));
    const roles = bytes ? JSON.parse(bytes).roles : {};
    const mapping = Object.fromEntries(context.schema.collections.map(c => [c.name, c.name]));
    Object.assign(mapping, roles);
    let recipe = queries[name];
    if (name === 'records') {
      const collection = options.collection;
      if (!context.schema.collections.some(c => c.name === collection)) fail('COLLECTION_UNKNOWN', 'records requires a configured --collection alias.');
      recipe = { needs: { [collection]: [] }, sql: () => `SELECT * FROM ${quote(collection)} ORDER BY name` };
    }
    if (!recipe) fail('QUERY_UNKNOWN', `Choose query: ${[...Object.keys(queries), 'records'].join(', ')}`);
    for (const [role, fields] of Object.entries(recipe.needs)) {
      const collection = context.schema.collections.find(c => c.name === mapping[role]);
      if (!collection || fields.some(field => !collection.fields.some(f => f.name === field))) fail('CONTRACT_INCOMPATIBLE', `Read schema/usage and resolve missing ${role} fields before this query.`);
    }
    const limit = options.limit ?? 1000;
    const result = attendant(context.temporary, ['query', '--sql', recipe.sql(mapping), '--limit', String(limit)]);
    if (name === 'matrix') {
      // Attendant's authorizer intentionally denies json_each virtual-table reads.
      // Expand only the declared arrays, with the same bounded output limit.
      const rows = [];
      let truncated = result.truncated;
      outer: for (const row of result.rows) {
        for (const noun of JSON.parse(row.nouns ?? '[]')) {
          if (rows.length >= limit) { truncated = true; break outer; }
          const { nouns, ...rest } = row;
          rows.push({ noun, ...rest });
        }
      }
      Object.assign(result, { rows, columns: ['noun', ...result.columns.filter(column => column !== 'nouns')], returned: rows.length, truncated });
    }
    return { ...result, query: name, semantics: 'Declared SQLite projection only; not agent review or native C99 analysis.' };
  });
}
export async function resolveTerm(root, term) {
  const result = await runQuery(root, 'glossary', { limit: 10000 });
  if (result.data.truncated) fail('TERM_LIMIT', 'Glossary exceeds 10000 rows; cannot resolve globally without complete scope.');
  const normalize = text => text.trim().replace(/\s+/gu, ' ').toLowerCase();
  const index = new Map();
  for (const row of result.data.rows) {
    const aliases = row.aliases === null ? [] : JSON.parse(row.aliases);
    for (const [field, text] of [['term', row.term], ...aliases.map(alias => ['aliases', alias])]) {
      if (typeof text !== 'string' || !normalize(text)) continue;
      const key = normalize(text);
      if (!index.has(key)) index.set(key, []);
      index.get(key).push({ ...row, field });
    }
  }
  const matches = index.get(normalize(term)) ?? [];
  return { ...result, data: { state: matches.length === 0 ? 'unknown' : matches.length === 1 ? 'resolved' : 'ambiguous', matches, collisions: [...index].filter(([, owners]) => owners.length > 1).map(([normalized, owners]) => ({ code: 'term.collision', normalized, owners })), normalization: 'trim/collapse-whitespace/Unicode-lowercase; no punctuation or plural inference' } };
}
