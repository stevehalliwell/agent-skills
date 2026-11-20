#!/usr/bin/env node
/** Convert shipped live collection frontmatter from legacy flow maps to block YAML.
 * Usage: node scripts/convert-workbench-frontmatter.mjs [--root REPO] [--apply] [--json]
 * Plan only by default. Scans immediate Markdown files in template collections and
 * both examples' game-design collections; never scans frozen results or experiments/.
 * Preserve metadata types/order, UTF-8, BOM, newline style and all bytes after header.
 * Supports flat flow maps with scalar/scalar-array values, including legacy bare
 * paths/keys. Refuses duplicate keys, nested containers and unsafe integer literals.
 * Validate entire plan before writing. Per-file atomic replacement, NOT a batch
 * transaction. Failures report completed paths; inspect before retry. No Attendant
 * projection changes, schema expansion, dependency installation or experiment runs.
 */
import { readFile, readdir, lstat, writeFile, rename, unlink } from 'node:fs/promises';
import { resolve, join, dirname, relative } from 'node:path';
import { fileURLToPath } from 'node:url';

const defaultRoot = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const help = `Usage: node scripts/convert-workbench-frontmatter.mjs [--root REPO] [--apply] [--json]
Plan only by default; --apply converts live collection files and shipped templates.
JSON stdout in all modes; diagnostics on stderr. Exit 0 success; 2 failure.
No recursive evidence/fixture conversion, metadata changes or body rewrites.
Failures include changed paths; completed writes remain. Safe rerun skips block YAML.
`;
const decoder = new TextDecoder('utf-8', { fatal: true, ignoreBOM: true });
function fail(message) { throw new Error(message); }

// Deliberately bounded legacy format, not a general YAML parser. JSON quotes and
// primitives plus bare technical names/paths cover shipped legacy flow headers.
export function parseFlow(header) {
  const tokens = [];
  const token = /\s+|"(?:[^"\\\x00-\x1f]|\\(?:["\\/bfnrt]|u[0-9a-fA-F]{4}))*"|[{}\[\]:,]|[^\s{}\[\]:,"]+/gy;
  let offset = 0;
  while (offset < header.length) {
    token.lastIndex = offset;
    const match = token.exec(header);
    if (!match) fail(`Unsupported flow token at character ${offset}`);
    offset = token.lastIndex;
    if (!/^\s+$/.test(match[0])) tokens.push(match[0]);
  }
  let index = 0;
  const take = expected => {
    if (tokens[index++] !== expected) fail(`Expected ${expected} in flow header`);
  };
  const scalar = () => {
    const value = tokens[index++];
    if (value?.startsWith('"')) return JSON.parse(value);
    if (['null', 'true', 'false'].includes(value)) return JSON.parse(value);
    if (/^-?(?:0|[1-9]\d*)(?:\.\d+)?(?:[eE][+-]?\d+)?$/.test(value ?? '')) {
      const number = Number(value);
      if (!Number.isFinite(number) || (Number.isInteger(number) && !Number.isSafeInteger(number)))
        fail(`Unsafe numeric literal: ${value}`);
      return number;
    }
    if (/^[A-Za-z_][A-Za-z0-9_./-]*$/.test(value ?? '')) return value;
    fail(`Unsupported scalar: ${value ?? 'end of header'}`);
  };
  const fields = new Map();
  take('{');
  while (tokens[index] !== '}') {
    const token = tokens[index++];
    const key = token?.startsWith('"') ? JSON.parse(token) : token;
    if (typeof key !== 'string' || !/^[A-Za-z_][A-Za-z0-9_-]*$/.test(key)) fail(`Unsupported key: ${key}`);
    if (fields.has(key)) fail(`Duplicate key: ${key}`);
    take(':');
    let value;
    if (tokens[index] === '[') {
      ++index;
      value = [];
      while (tokens[index] !== ']') {
        value.push(scalar());
        if (tokens[index] === ']') break;
        take(',');
        if (tokens[index] === ']') fail('Trailing array comma is unsupported');
      }
      take(']');
    } else value = scalar();
    fields.set(key, value);
    if (tokens[index] === '}') break;
    take(',');
    if (tokens[index] === '}') fail('Trailing map comma is unsupported');
  }
  take('}');
  if (index !== tokens.length) fail('Unexpected content after flow header');
  if (!fields.size) fail('Empty flow map cannot become a block mapping');
  return fields;
}

export function convert(bytes) {
  const source = decoder.decode(bytes);
  const header = source.match(/^(\uFEFF?---\r?\n)([\s\S]*?)(\r?\n---(?:\r?\n|$))/);
  if (!header || !header[2].trimStart().startsWith('{')) return null;
  const fields = parseFlow(header[2]);
  const newline = header[1].endsWith('\r\n') ? '\r\n' : '\n';
  const scalar = value => Object.is(value, -0) ? '-0' : JSON.stringify(value);
  const lines = [];
  for (const [key, value] of fields) {
    if (Array.isArray(value) && value.length) {
      lines.push(`${key}:`, ...value.map(item => `  - ${scalar(item)}`));
    } else lines.push(`${key}: ${Array.isArray(value) ? '[]' : scalar(value)}`);
  }
  const body = source.slice(header[0].length);
  const result = Buffer.from(header[1] + lines.join(newline) + header[3] + body, 'utf8');
  // Body preservation is checked independently from the serializer's output text.
  const oldBody = bytes.subarray(Buffer.byteLength(header[0], 'utf8'));
  if (!result.subarray(result.length - oldBody.length).equals(oldBody)) fail('Body bytes changed');
  return result;
}

async function ordinary(path, directory = false) {
  const stat = await lstat(path);
  if (stat.isSymbolicLink() || !(directory ? stat.isDirectory() : stat.isFile()))
    fail(`Expected ordinary ${directory ? 'directory' : 'file'}: ${path}`);
}
async function candidates(root) {
  // Check ancestors too: an ordinary file beneath a linked directory is not safe.
  const skill = join(root, 'skills', 'game-design-workbench');
  for (const path of [root, join(root, 'skills'), skill, join(skill, 'examples')]) await ordinary(path, true);
  const groups = [join(skill, 'templates')];
  for (const name of ['crate-puzzle', 'little-plumber-guy-nes']) {
    const project = join(skill, 'examples', name);
    await ordinary(project, true);
    groups.push(join(project, 'game-design'));
  }
  const paths = [];
  for (const group of groups) {
    await ordinary(group, true);
    for (const collection of (await readdir(group, { withFileTypes: true })).sort((a, b) => a.name.localeCompare(b.name))) {
      if (collection.isSymbolicLink()) fail(`Refuse linked collection: ${join(group, collection.name)}`);
      if (!collection.isDirectory()) continue;
      const directory = join(group, collection.name);
      for (const entry of (await readdir(directory, { withFileTypes: true })).sort((a, b) => a.name.localeCompare(b.name))) {
        if (!entry.name.endsWith('.md')) continue;
        const path = join(directory, entry.name);
        await ordinary(path);
        paths.push(path);
      }
    }
  }
  return paths;
}

async function main() {
  const changed = [];
  let apply = false, root = defaultRoot;
  try {
    for (let index = 2; index < process.argv.length; ++index) {
      const argument = process.argv[index];
      if (argument === '--help' || argument === '-h') { process.stdout.write(help); return; }
      if (argument === '--apply') apply = true;
      else if (argument === '--json') { /* Output is already JSON. */ }
      else if (argument === '--root') {
        const value = process.argv[++index];
        if (!value || value.startsWith('--')) fail('--root requires a repository path');
        root = resolve(value);
      } else fail(`Unknown argument: ${argument}`);
    }
    const plan = [];
    for (const path of await candidates(root)) {
      const before = await readFile(path);
      let after;
      try { after = convert(before); } catch (error) { fail(`${relative(root, path)}: ${error.message}`); }
      if (after) plan.push({ path, before, after });
    }
    if (apply) for (const item of plan) {
      await ordinary(item.path);
      if (!(await readFile(item.path)).equals(item.before)) fail(`Source changed after planning: ${item.path}`);
      const temporary = `${item.path}.frontmatter-${process.pid}.tmp`;
      await writeFile(temporary, item.after, { flag: 'wx' });
      try {
        if (!(await readFile(item.path)).equals(item.before)) fail(`Source changed before replacement: ${item.path}`);
        await rename(temporary, item.path);
        changed.push(relative(root, item.path).replaceAll('\\', '/'));
      } catch (error) {
        await unlink(temporary);
        throw error;
      }
    }
    console.log(JSON.stringify({ ok: true, mode: apply ? 'apply' : 'plan', proposed: plan.length,
      changed, files: plan.map(item => relative(root, item.path).replaceAll('\\', '/')) }));
  } catch (error) {
    console.log(JSON.stringify({ ok: false, partial: changed.length > 0, changed,
      error: { code: 'frontmatter.conversion', message: error.message } }));
    console.error(`frontmatter conversion: ${error.message}`);
    process.exitCode = 2;
  }
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) await main();
