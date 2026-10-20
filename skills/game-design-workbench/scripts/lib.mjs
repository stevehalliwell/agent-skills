import { open, readdir, lstat, realpath, mkdir, writeFile, rename, unlink } from 'node:fs/promises';
import { resolve, relative, join, dirname, isAbsolute } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createHash, randomBytes } from 'node:crypto';
import { spawnSync } from 'node:child_process';
export const home = resolve(dirname(fileURLToPath(import.meta.url)), '..');
export const hash = bytes => createHash('sha256').update(bytes).digest('hex');
export const slash = value => value.replaceAll('\\', '/');
export function fail(code, message, data) { throw Object.assign(new Error(message), { code, data }); }
export async function optional(path) {
  let handle;
  try {
    handle = await open(path, 'r');
    const info = await handle.stat();
    const limit = 2 * 1024 * 1024;
    if (!info.isFile() || info.size > limit) fail('SOURCE_LIMIT', `Expected regular file of at most 2 MiB: ${path}`);
    const buffer = Buffer.alloc(Math.min(info.size + 1, limit + 1));
    let length = 0;
    while (length < buffer.length) {
      const result = await handle.read(buffer, length, buffer.length - length, null);
      if (!result.bytesRead) break;
      length += result.bytesRead;
    }
    if (length > info.size) fail('SOURCE_CHANGED', `File grew during read: ${path}`);
    return Buffer.from(buffer.subarray(0, length));
  } catch (error) { if (error.code === 'ENOENT') return null; throw error; }
  finally { await handle?.close(); }
}
export async function safePath(root, name) {
  const path = resolve(root, name);
  const rel = relative(root, path);
  if (rel === '..' || rel.startsWith(`..${process.platform === 'win32' ? '\\' : '/'}`) || isAbsolute(rel)) fail('PATH_OUTSIDE_PROJECT', `Path escapes project: ${name}`);
  let cursor = root;
  for (const part of rel.split(/[\\/]/).filter(Boolean)) {
    cursor = join(cursor, part);
    try { if ((await lstat(cursor)).isSymbolicLink()) fail('SYMLINK_UNSUPPORTED', `Refusing symlink/reparse path: ${name}`); }
    catch (error) { if (error.code === 'ENOENT') break; throw error; }
  }
  return path;
}
export async function projectRoot(path) { return realpath(resolve(path)); }
export function attendant(root, args) {
  const child = spawnSync(process.execPath, [join(home, 'vendor/attendant/scripts/attendant.mjs'), ...args, '--project', root], { encoding: 'utf8', timeout: 30000, maxBuffer: 16 * 1024 * 1024, windowsHide: true });
  if (child.error) fail('ATTENDANT_RUNTIME', child.error.message);
  let data;
  try { data = JSON.parse(child.stdout); } catch { fail('ATTENDANT_RUNTIME', child.stderr.trim() || 'Attendant did not return JSON.'); }
  if (child.status !== 0) fail('ATTENDANT_REJECTED', child.stderr.trim() || 'Attendant rejected operation.', data);
  return data;
}
export async function collections(root) {
  const path = await safePath(root, '.pi/attendant.tables');
  const bytes = await optional(path);
  if (!bytes) return [];
  const result = [];
  for (const raw of bytes.toString('utf8').split(/\r?\n/)) {
    const line = raw.trim();
    if (!line || line.startsWith('#')) continue;
    const match = /^(.*?)\s+as\s+(.+)$/.exec(line);
    const directory = match ? match[1].trim() : line;
    const alias = match ? match[2].trim() : directory.replaceAll('\\', '/').split('/').filter(Boolean).at(-1);
    if (!/^[A-Za-z0-9][A-Za-z0-9_-]*$/.test(alias) || result.some(item => item.alias === alias)) fail('CONFIG_INVALID', `Invalid/duplicate collection alias: ${alias}`);
    const absolute = await safePath(root, directory);
    if (absolute === root) fail('CONFIG_INVALID', 'Collections must use dedicated in-project directories, not project root.');
    result.push({ alias, directory: slash(relative(root, absolute)), absolute });
  }
  return result;
}
export async function walk(root, directory, files = []) {
  for (const entry of (await readdir(directory, { withFileTypes: true })).sort((a, b) => a.name.localeCompare(b.name))) {
    const path = await safePath(root, join(directory, entry.name));
    if (entry.isDirectory()) await walk(root, path, files);
    else if (entry.isFile()) {
      const info = await lstat(path);
      if (info.size > 2 * 1024 * 1024 || files.length >= 5000 || files.reduce((total, file) => total + file.bytes.length, 0) + info.size > 32 * 1024 * 1024) fail('SOURCE_LIMIT', 'Source limits: 5000 files, 2 MiB per file, 32 MiB total.');
      const bytes = await optional(path);
      if (bytes === null) fail('SOURCE_CHANGED', `Source disappeared: ${path}`);
      files.push({ path: slash(relative(root, path)), bytes });
    } else fail('SOURCE_TYPE', `Unsupported source entry: ${path}`);
  }
  return files;
}
export async function bundle() {
  const files = [];
  for (const folder of ['scripts', 'templates', 'references', 'vendor']) {
    const info = await lstat(join(home, folder)).catch(error => { if (error.code === 'ENOENT') return null; throw error; });
    if (!info) continue;
    if (!info.isDirectory()) fail('PACKAGE_INVALID', `Package folder is not a directory: ${folder}`);
    await walk(home, join(home, folder), files);
  }
  for (const name of ['README.md', 'SKILL.md']) { const bytes = await optional(join(home, name)); if (bytes) files.push({ path: name, bytes }); }
  return files.filter(file => !['scripts/build-example.mjs', 'scripts/fixture-data.mjs'].includes(file.path)).sort((a, b) => a.path.localeCompare(b.path));
}
export async function snapshot(root, includes = []) {
  const files = [];
  for (const name of ['.pi/attendant.tables', 'AGENTS.md', 'tools/game-design-workbench/manifest.json']) {
    const path = await safePath(root, name); const bytes = await optional(path); if (bytes) files.push({ path: name, bytes });
  }
  for (const collection of await collections(root)) await walk(root, collection.absolute, files);
  for (const name of includes) { const path = await safePath(root, name); const bytes = await optional(path); if (bytes === null) fail('SOURCE_MISSING', `Missing included source: ${name}`); files.push({ path: slash(relative(root, path)), bytes }); }
  const unique = [...new Map(files.map(file => [file.path, file])).values()].sort((a, b) => a.path.localeCompare(b.path));
  if (unique.reduce((sum, file) => sum + file.bytes.length, 0) > 32 * 1024 * 1024) fail('SOURCE_LIMIT', 'Snapshot exceeds 32 MiB source limit.');
  const sources = unique.map(file => ({ path: file.path, sha256: hash(file.bytes) }));
  const tool_hash = hash((await bundle()).map(file => `${file.path}:${hash(file.bytes)}`).join('\n'));
  return { files: unique, metadata: { schema_version: 1, fingerprint: hash(JSON.stringify({ sources, tool_hash })), sources, tool_hash, includes } };
}
export async function atomicWrite(root, name, bytes, expected = null) {
  const path = await safePath(root, name);
  const current = await optional(path);
  if ((current ? hash(current) : null) !== expected) fail('SOURCE_CHANGED', `Changed destination: ${name}`);
  await mkdir(dirname(path), { recursive: true });
  const temporary = `${path}.gd-${randomBytes(6).toString('hex')}`;
  await writeFile(temporary, bytes, { flag: 'wx' });
  try {
    const latest = await optional(path);
    if ((latest ? hash(latest) : null) !== expected) fail('SOURCE_CHANGED', `Changed destination: ${name}`);
    await rename(temporary, path);
  } finally { await unlink(temporary).catch(error => { if (error.code !== 'ENOENT') throw error; }); }
}
