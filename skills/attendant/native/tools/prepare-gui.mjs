#!/usr/bin/env node
// Explicit dependency acquisition only. Build never downloads or installs tools.
import { spawnSync } from 'node:child_process';
import { mkdirSync, existsSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { patchRichMd, richMdPatchedFiles } from './patch-richmd.mjs';
process.chdir(resolve(dirname(fileURLToPath(import.meta.url)), '..'));
const verifyOnly = process.argv[2] === '--verify';
if (process.argv.length > (verifyOnly ? 3 : 2)) throw new Error('Usage: node tools/prepare-gui.mjs [--verify]');
function git(args, cwd = process.cwd()) {
  const result = spawnSync('git', args, { cwd, encoding: 'utf8' });
  if (result.status !== 0) throw new Error(result.stderr || result.error?.message || 'git failed');
  return result.stdout.trim();
}
const dependencies = [
  ['cimgui', 'https://github.com/cimgui/cimgui.git', '2a6a93fccd10f275ee9206167dc9698f6f00fac1'],
  ['cimgui/imgui', 'https://github.com/ocornut/imgui.git', 'c1aaac65e896a45f81f47a4d68f9959e0f86c859'],
  ['richmd', 'https://github.com/pthom/imgui_rich_md.git', '609039721f838108251520079fff79011cf46659'],
  ['richmd/external/md4c', 'https://github.com/mity/md4c.git', '481fbfbdf72daab2912380d62bb5f2187d438408'],
  ['luajit', 'https://github.com/LuaJIT/LuaJIT.git', 'a4c064539ea931274d407e1249b4bb575424a0f4'],
];
for (const [name, url, commit] of dependencies) {
  const path = resolve('build/deps', name);
  if (!existsSync(resolve(path, '.git'))) {
    if (verifyOnly) throw new Error(`Missing dependency; run node tools/prepare-gui.mjs: ${name}`);
    mkdirSync(path, { recursive: true });
    git(['init', path]);
    git(['remote', 'add', 'origin', url], path);
    git(['fetch', '--depth', '1', 'origin', commit], path);
    git(['checkout', '--detach', 'FETCH_HEAD'], path);
  }
  if (git(['rev-parse', 'HEAD'], path) !== commit) throw new Error(`Wrong revision: ${path}`);
  const allowed = new Set(name === 'cimgui' ? ['imgui', 'cimgui.h', 'cimgui.cpp',
    ...['constants', 'definitions', 'structs_and_enums'].flatMap(base => ['json', 'lua'].map(extension => `generator/output/${base}.${extension}`)),
    'generator/output/overloads.txt'] : name === 'cimgui/imgui' ? ['imgui.h'] : name === 'richmd' ? richMdPatchedFiles : []);
  for (const changed of git(['diff', 'HEAD', '--name-only'], path).split('\n').filter(Boolean))
    if (!allowed.has(changed)) throw new Error(`Unexpected tracked dependency edit: ${name}/${changed}`);
}
// ImGui 1.93 unconditionally redefines IM_NODEBUGSTEP, defeating cimgui's existing
// -DIM_NODEBUGSTEP="". Guard the attribute block; no struct/layout/API changes.
const header = 'build/deps/cimgui/imgui/imgui.h';
const original = git(['show', 'HEAD:imgui.h'], resolve('build/deps/cimgui/imgui'));
const block = /#if \(defined\(__clang__\)[\s\S]*?#define IM_NODEBUGSTEP\s*\n#endif/;
const match = original.match(block);
if (!match || !match[0].includes('[[gnu::artificial]]')) throw new Error('ImGui attribute patch anchor changed');
const patched = original.replace(block, `#ifndef IM_NODEBUGSTEP\n${match[0]}\n#endif`);
const current = readFileSync(header, 'utf8').trimEnd().replaceAll('\r\n', '\n');
if (current !== original && current !== patched) throw new Error('Unexpected local ImGui header edits; refusing overwrite');
if (verifyOnly && current !== patched) throw new Error('Missing expected ImGui attribute patch; run node tools/prepare-gui.mjs');
if (!verifyOnly) writeFileSync(header, patched + '\n');
patchRichMd(verifyOnly);
writeFileSync('build/dependencies.json', JSON.stringify({ dependencies, patch: 'Guard IM_NODEBUGSTEP; strict original-input RichMd source-selection snapshot with explicit run provenance' }, null, 2) + '\n');
console.log(verifyOnly ? 'Dependency revisions and tracked edits verified offline.' : 'Pinned sources ready. Next: bash tools/build-gui-stack.sh');
