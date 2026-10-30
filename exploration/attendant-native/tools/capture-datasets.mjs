// Capture active Attendant Markdown without running commands in source projects.
// Usage: node tools/capture-datasets.mjs <project> [...]
// Node is analysis infrastructure only. Output: JSON summary with manifest path.
// Creates a unique ignored build/real-data/capture-* directory; never overwrites.
// Only project-local collections with regular, immediate Markdown files supported.
// Refuse symbolic links/external collections instead of retaining live references.
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { cpSync, mkdirSync, mkdtempSync, readFileSync, realpathSync, writeFileSync } from 'node:fs';
import { basename, dirname, join, resolve } from 'node:path';
import { inventory, localPath } from './dataset-files.mjs';
import { fileURLToPath } from 'node:url';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const args = process.argv.slice(2);
if (!args.length || args.includes('--help')) {
  console.log(JSON.stringify({ usage: 'node tools/capture-datasets.mjs <project> [...]', output: 'JSON manifest and unique build/real-data/capture-* directory', safety: 'read originals only; no source DBs, Git folders, external paths or links copied; separate pristine/mjs/cjit/zig copies' }));
  process.exit(args.includes('--help') ? 0 : 2);
}
try {
  const projects = args.map(path => realpathSync(resolve(path)));
  const labels = projects.map(path => basename(path).toLowerCase());
  assert.equal(new Set(labels).size, labels.length, 'Source project basenames must be unique');
  // Preflight all inputs before creating any output.
  const snapshots = projects.map(project => ({ source: project, ...inventory(project) }));
  const parent = join(root, 'build/real-data'); mkdirSync(parent, { recursive: true });
  const directory = mkdtempSync(join(parent, 'capture-'));
  const manifest = { capturedAt: new Date().toISOString(), directory, sourceStableDuringCapture: false, datasets: [] };
  for (const snapshot of snapshots) {
    const label = basename(snapshot.source), pristine = join(directory, label, 'pristine');
    for (const file of snapshot.files) {
      const bytes = readFileSync(localPath(snapshot.source, file.path));
      assert.equal(createHash('sha256').update(bytes).digest('hex'), file.sha256, `Source changed during capture: ${file.path}`);
      const target = join(pristine, file.path); mkdirSync(dirname(target), { recursive: true });
      writeFileSync(target, bytes, { flag: 'wx' });
    }
    assert.deepEqual(inventory(pristine), { collections: snapshot.collections, files: snapshot.files }, 'Pristine copy differs');
    const working = {};
    for (const variant of ['mjs', 'cjit', 'zig']) {
      working[variant] = join(directory, label, variant);
      cpSync(pristine, working[variant], { recursive: true, errorOnExist: true, force: false });
      assert.deepEqual(inventory(working[variant]), inventory(pristine), `Working copy differs: ${variant}`);
    }
    manifest.datasets.push({ ...snapshot, pristine, working });
  }
  // Detect concurrent changes; this is not an atomic cross-file snapshot guarantee.
  for (const snapshot of snapshots) assert.deepEqual(inventory(snapshot.source), { collections: snapshot.collections, files: snapshot.files }, `Source changed during capture: ${snapshot.source}`);
  manifest.sourceStableDuringCapture = true;
  const manifestPath = join(directory, 'manifest.json');
  writeFileSync(manifestPath, JSON.stringify(manifest, null, 2) + '\n', { flag: 'wx' });
  console.log(JSON.stringify({ directory, manifestPath, sourceStableDuringCapture: true, datasets: manifest.datasets.map(dataset => ({
    source: dataset.source, pristine: dataset.pristine, working: dataset.working, collections: dataset.collections,
    files: dataset.files.length, bytes: dataset.files.reduce((sum, file) => sum + file.bytes, 0), largestFileBytes: dataset.files.reduce((largest, file) => Math.max(largest, file.bytes), 0)
  })) }));
} catch (error) {
  const result = { error: { code: 'dataset-capture', message: error.message } };
  console.log(JSON.stringify(result)); console.error(result.error.message); process.exitCode = 1;
}
