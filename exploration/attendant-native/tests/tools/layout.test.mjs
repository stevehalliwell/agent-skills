// Read-only layout checks. Keep runtime review separate from test and experiment code.
import test from 'node:test';
import assert from 'node:assert/strict';
import { existsSync, readFileSync, readdirSync } from 'node:fs';
import { dirname, join, resolve, relative } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const text = path => readFileSync(join(root, path), 'utf8');
function files(folder) {
  return readdirSync(join(root, folder), { withFileTypes: true }).flatMap(entry =>
    entry.isDirectory() ? files(join(folder, entry.name)) : [join(folder, entry.name)]);
}
test('applications own separate mains; shared core has no main or CLI inclusion switch', () => {
  const entries = files('apps').filter(path => path.endsWith('.c') && /\bint main\(/.test(text(path)));
  assert.deepEqual(entries.map(path => path.replaceAll('\\', '/')).sort(), ['apps/cli/main.c', 'apps/gui/main.c']);
  for (const path of files('src').filter(path => /\.[ch]$/.test(path))) {
    assert.doesNotMatch(text(path), /\bint main\(|ATTENDANT_EMBEDDED/);
    assert.doesNotMatch(text(path), /#include .*apps\/cli/);
  }
  assert(!existsSync(join(root, 'projection.c')));
  assert(!existsSync(join(root, 'gui/main.c')));
});
test('GUI drivers, synthetic input and completion assertions live outside application source', () => {
  const main = text('apps/gui/main.c');
  assert.doesNotMatch(main, /ImGuiIO_Add|REVEAL_CHECK|Undo fixture|--exercise-selection|Selection exercise did not complete/);
  assert.match(text('tests/gui/driver.h'), /Undo fixture/);
  assert.match(text('tests/gui/complete.inc'), /Selection exercise did not complete/);
  const lines = main.split('\n');
  for (let i = 0; i < lines.length; i++) if (/^#include .*tests\/gui\//.test(lines[i])) {
    assert.equal(lines[i - 1], '#ifdef ATTENDANT_GUI_TESTING', 'Test source is absent from production preprocessing');
    assert.equal(lines[i + 1], '#endif');
  }
});
test('feedback fault injection stays in test source and is guarded out of production', () => {
  const api = text('src/native-api.c');
  assert.doesNotMatch(api, /GetEnvironmentVariableA|FILE_APPEND_DATA/);
  assert(api.includes('#ifdef ATTENDANT_TEST_FAULTS\n#include "../tests/core/feedback-faults.h"\n#else'));
  assert.match(text('tests/core/feedback-faults.h'), /ATTENDANT_FEEDBACK_FAULT/);
});
test('moved modules retain resolvable relative imports and native-root discovery', () => {
  for (const folder of ['tools', 'tests/core', 'tests/gui', 'tests/tools', 'experiments/sqlite']) {
    for (const path of files(folder).filter(path => path.endsWith('.mjs'))) {
      const source = text(path);
      for (const match of source.matchAll(/from ['"](\.[^'"]+)['"]/g)) {
        assert(existsSync(resolve(root, dirname(path), match[1])), `${path}: ${match[1]}`);
      }
      if (source.includes('const root =')) {
        const depth = relative(dirname(join(root, path)), root).replaceAll('\\', '/');
        assert(source.includes(`const root = resolve(dirname(fileURLToPath(import.meta.url)), '${depth}')`), path);
      }
    }
  }
});
