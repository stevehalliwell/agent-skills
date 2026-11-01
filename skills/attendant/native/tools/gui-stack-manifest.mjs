#!/usr/bin/env node
// Called only after successful build. Bind actual source contents/toolchain to DLL.
import { readFileSync, writeFileSync, statSync, existsSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
process.chdir(resolve(dirname(fileURLToPath(import.meta.url)), '..'));
function run(command, args, cwd = process.cwd()) {
  const result = spawnSync(command, args, { cwd, encoding: 'utf8' });
  if (result.status !== 0) throw new Error(result.stderr || result.error?.message || 'Command failed');
  return result.stdout.trim();
}
const sha256 = path => createHash('sha256').update(readFileSync(path)).digest('hex');
const dependencies = JSON.parse(readFileSync('build/dependencies.json', 'utf8'));
const paths = new Set(['vendor/gui/richmd_c.cpp', 'vendor/gui/richmd_c.h', 'vendor/gui/preview-selection.cpp.inc', 'tools/build-gui-stack.sh', 'tools/prepare-gui.mjs', 'tools/patch-richmd.mjs', 'tools/fix-generated.mjs', 'tools/gui-stack-manifest.mjs']);
for (const [name] of dependencies.dependencies) {
  const folder = `build/deps/${name}`;
  for (const entry of run('git', ['ls-files', '-z'], resolve(folder)).split('\0').filter(Boolean)) {
    const path = `${folder}/${entry}`;
    if (existsSync(path) && statSync(path).isFile()) paths.add(path);
  }
}
for (const path of ['luajit.h', 'host/buildvm_arch.h', 'lj_bcdef.h', 'lj_ffdef.h', 'lj_libdef.h', 'lj_recdef.h', 'lj_folddef.h', 'jit/vmdef.lua'])
  paths.add(`build/deps/luajit/src/${path}`);
const files = [...paths].sort().map(path => ({ path, sha256: sha256(path) }));
writeFileSync('build/build-manifest.json', JSON.stringify({
  builtAt: new Date().toISOString(), zig: run(process.env.ZIG || 'zig', ['version']), generatorZig: run('zig', ['version']),
  dependencies, files, sourceTreeSha256: createHash('sha256').update(JSON.stringify(files)).digest('hex'),
  dllSha256: sha256('build/bin/cimgui_richmd.dll'),
  generatorSha256: sha256('build/bin/luajit.exe'),
}, null, 2) + '\n');
