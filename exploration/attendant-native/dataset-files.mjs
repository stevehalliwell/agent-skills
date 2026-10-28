// Shared read-only inventory for captured, project-local Attendant datasets.
// Reject external collection paths and links before any CLI can follow them.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { lstatSync, readFileSync, readdirSync } from 'node:fs';
import { basename, isAbsolute, join, relative, resolve, sep } from 'node:path';
export function localPath(project, path) {
  const target = resolve(project, path), local = relative(project, target);
  assert.ok(local && !isAbsolute(local) && local !== '..' && !local.startsWith('..' + sep), `Path must stay inside source project: ${path}`);
  let current = project;
  for (const part of local.split(sep)) {
    current = join(current, part);
    assert.ok(!lstatSync(current).isSymbolicLink(), `Refusing symbolic link: ${current}`);
  }
  return target;
}
export function inventory(project) {
  const configPath = localPath(project, '.pi/attendant.tables');
  assert.ok(lstatSync(configPath).isFile(), `Not a config file: ${configPath}`);
  const config = readFileSync(configPath), collections = [], files = ['.pi/attendant.tables'];
  const directories = new Set(), names = new Set();
  for (const line of new TextDecoder('utf-8', { fatal: true }).decode(config).split(/\r?\n/).map(line => line.trim())) {
    if (!line || line.startsWith('#')) continue;
    const match = /^(.*?)\s+as\s+([^\s/\\]+)$/.exec(line);
    const directory = match ? match[1] : line;
    assert.ok(!isAbsolute(directory), `Capture supports relative collection paths only: ${directory}`);
    const source = localPath(project, directory), local = relative(project, source);
    assert.ok(lstatSync(source).isDirectory(), `Not a collection directory: ${source}`);
    assert.ok(!directories.has(source.toLowerCase()), `Repeated collection directory: ${directory}`);
    directories.add(source.toLowerCase());
    const name = match ? match[2] : basename(source);
    assert.ok(!names.has(name.toLowerCase()), `Repeated collection name: ${name}`); names.add(name.toLowerCase());
    let records = 0;
    for (const entry of readdirSync(source, { withFileTypes: true }).sort((a, b) => a.name < b.name ? -1 : a.name > b.name ? 1 : 0)) {
      assert.ok(!entry.isSymbolicLink(), `Refusing collection link: ${join(source, entry.name)}`);
      if (!entry.isFile() || !entry.name.endsWith('.md')) continue;
      files.push(join(local, entry.name));
      if (!entry.name.startsWith('.')) records++;
    }
    assert.ok(files.includes(join(local, '.schema.md')), `Missing schema: ${source}`);
    collections.push({ name, directory: local, records });
  }
  const entries = files.sort().map(path => {
    const bytes = readFileSync(localPath(project, path));
    return { path, bytes: bytes.length, sha256: createHash('sha256').update(bytes).digest('hex') };
  });
  return { collections, files: entries };
}
