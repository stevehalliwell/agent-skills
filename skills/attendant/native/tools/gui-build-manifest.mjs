// Build/check provenance. GUI runtime does not use Node or these manifests.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { readFileSync, readdirSync, writeFileSync } from 'node:fs';
import { dirname, join, relative, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const stack = root;
const variant = process.env.ATTENDANT_GUI_VARIANT || '';
assert(!variant || /^[a-z0-9][a-z0-9-]*$/.test(variant), 'Invalid ATTENDANT_GUI_VARIANT');
export const guiDirectory = `build/gui${variant ? '-' + variant : ''}`;
const sha256 = path => createHash('sha256').update(readFileSync(path)).digest('hex');
function stackManifest() {
  const manifest = JSON.parse(readFileSync(join(stack, 'build/build-manifest.json')));
  for (const file of manifest.files) assert.equal(sha256(join(stack, file.path)), file.sha256, `GUI stack input changed: ${file.path}`);
  assert.equal(sha256(join(stack, 'build/bin/cimgui_richmd.dll')), manifest.dllSha256, 'GUI stack DLL changed');
  return { sha256: sha256(join(stack, 'build/build-manifest.json')), dllSha256: manifest.dllSha256, dependencies: manifest.dependencies };
}
function inputFiles() {
  const paths = ['build-gui.sh', 'tools/build-inputs.sh', 'tools/gui-build-manifest.mjs'];
  function walk(folder) {
    for (const entry of readdirSync(join(root, folder), { withFileTypes: true })) {
      const path = join(folder, entry.name);
      if (entry.isDirectory()) walk(path); else if (/\.(c|h|inc)$/.test(path)) paths.push(path);
    }
  }
  for (const folder of ['src', 'apps/gui', 'tests/gui', 'tests/core', 'vendor']) walk(folder);
  return paths.sort().map(path => ({ path: path.replaceAll('\\', '/'), sha256: sha256(join(root, path)) }));
}
export function verifyBuild(compiler, testing = false) {
  assert(['cjit', 'zig'].includes(compiler));
  const name = `attendant-gui-${compiler}${testing ? '-check' : ''}`;
  const manifest = JSON.parse(readFileSync(join(root, `${guiDirectory}/${name}.json`)));
  assert.equal(manifest.compiler, compiler); assert.equal(manifest.testing, testing);
  assert.deepEqual(inputFiles(), manifest.inputs, 'GUI build inputs changed; rebuild before acceptance');
  assert.deepEqual(stackManifest(), manifest.stack, 'GUI stack manifest changed; rebuild GUI');
  assert.equal(sha256(join(root, `${guiDirectory}/cimgui_richmd.dll`)), manifest.stack.dllSha256, 'Copied GUI DLL changed');
  for (const file of manifest.binaries) assert.equal(sha256(join(root, file.path)), file.sha256, `GUI binary changed: ${file.path}`);
  return manifest;
}
function writeManifest(compiler, testing) {
  const name = `attendant-gui-${compiler}${testing ? '-check' : ''}`;
  const executable = process.env[compiler === 'zig' ? 'ZIG' : 'CJIT'] || compiler;
  const version = spawnSync(executable, [compiler === 'zig' ? 'version' : '--help'], { encoding: 'utf8' });
  assert.equal(version.status, 0, 'Compiler version probe failed');
  const binaries = [join(root, `${guiDirectory}/${name}.exe`)];
  if (testing) binaries.push(join(root, `${guiDirectory}/model-check-${compiler}.exe`));
  const manifest = { builtAt: new Date().toISOString(), compiler, testing, compilerCommand: executable,
    version: { status: version.status, text: (version.stdout + version.stderr).slice(0, 1000) },
    inputs: inputFiles(), stack: stackManifest(), binaries: binaries.map(path => ({ path: relative(root, path).replaceAll('\\', '/'), sha256: sha256(path) })) };
  assert.equal(sha256(join(root, `${guiDirectory}/cimgui_richmd.dll`)), manifest.stack.dllSha256);
  writeFileSync(join(root, `${guiDirectory}/${name}.json`), JSON.stringify(manifest, null, 2) + '\n');
}
if (process.argv[1] && pathToFileURL(resolve(process.argv[1])).href === import.meta.url) {
  const [compiler, flag] = process.argv.slice(2);
  if (!['cjit', 'zig'].includes(compiler) || (flag && flag !== '--check')) throw new Error('Usage: node tools/gui-build-manifest.mjs cjit|zig [--check]');
  writeManifest(compiler, flag === '--check');
}
