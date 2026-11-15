// Run: node --test skills/game-design-workbench/test/frontmatter-migration.test.mjs
import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, readFile, writeFile, rm, symlink } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { convert, parseFlow } from '../../../scripts/convert-workbench-frontmatter.mjs';

const script = fileURLToPath(new URL('../../../scripts/convert-workbench-frontmatter.mjs', import.meta.url));
const source = '---\n{"name":"fixture","status":"accepted"}\n---\nOriginal body.\n';
function invoke(args, status = 0) {
  const result = spawnSync(process.execPath, [script, ...args], { encoding: 'utf8', timeout: 10000 });
  assert.equal(result.status, status, result.stderr || result.error?.message);
  return result;
}
async function fixture(t) {
  const root = await mkdtemp(join(tmpdir(), 'gd-frontmatter-'));
  t.after(() => rm(root, { recursive: true, force: true }));
  const home = join(root, 'skills/game-design-workbench');
  const folders = ['templates/challenges', 'examples/crate-puzzle/game-design/challenges',
    'examples/little-plumber-guy-nes/game-design/challenges'];
  for (const name of folders) await mkdir(join(home, name), { recursive: true });
  const paths = folders.map(name => join(home, name, 'fixture.md'));
  for (const path of paths) await writeFile(path, source, 'utf8');
  return { root, home, paths };
}

test('flat legacy maps preserve scalar types, arrays, bare paths and keys', () => {
  const header = '{"name":"yes", "enabled":false,"count":3,"ratio":0.25,"optional":null,"empty":[], context_axes:[axes/Contact], program:experiments/route-cost/run.c}';
  assert.deepEqual([...parseFlow(header)], [['name', 'yes'], ['enabled', false], ['count', 3], ['ratio', 0.25],
    ['optional', null], ['empty', []], ['context_axes', ['axes/Contact']], ['program', 'experiments/route-cost/run.c']]);
  const output = convert(Buffer.from(`---\n${header}\n---\nBody\n`)).toString('utf8');
  assert.equal(output, '---\nname: "yes"\nenabled: false\ncount: 3\nratio: 0.25\noptional: null\nempty: []\ncontext_axes:\n  - "axes/Contact"\nprogram: "experiments/route-cost/run.c"\n---\nBody\n');
  assert.equal(convert(Buffer.from(output)), null);
});

test('conversion preserves BOM, CRLF, quoted strings and exact body bytes', () => {
  const body = Buffer.from('\r\n# café\r\n```json\r\n{"not":"frontmatter"}\r\n```\r\n', 'utf8');
  const header = '\uFEFF---\r\n' + JSON.stringify({ desc: 'quotes " slash \\ colon: # é', created_at: '2026-10-19T00:00:00.000Z', label: 'false', zero: 0 }) + '\r\n---\r\n';
  const output = convert(Buffer.concat([Buffer.from(header, 'utf8'), body]));
  assert.ok(output.toString('utf8').startsWith('\uFEFF---\r\n'));
  assert.ok(output.subarray(output.length - body.length).equals(body));
  assert.match(output.toString('utf8'), /label: "false"\r\nzero: 0/);
  assert.equal(convert(Buffer.from('No frontmatter\n')), null);
  assert.equal(convert(Buffer.from('---\nname: fixture\n---\nBody')), null);
  assert.throws(() => convert(Buffer.from([0xff])), /encoded data/);
});

test('refuse duplicate fields, nested values, unsafe numbers and malformed flow', () => {
  for (const header of ['{"a":1,"a":2}', '{"a":{}}', '{"a":[[]]}', '{"a":9007199254740993}',
    '{"a":1e100}', '{"a":1,}', '{"a":[1,]}', '{"a":', '{}', '{"a":"unterminated}']) {
    assert.throws(() => parseFlow(header), undefined, header);
  }
  assert.ok(Object.is(parseFlow('{"value":-0}').get('value'), -0));
  assert.match(convert(Buffer.from('---\n{"value":-0}\n---\n')).toString('utf8'), /value: -0/);
});

test('CLI help and structured failures require no terminal interaction', () => {
  assert.match(invoke(['--help']).stdout, /Plan only by default/);
  for (const args of [['--unknown'], ['--root']]) {
    const result = invoke(args, 2);
    const value = JSON.parse(result.stdout);
    assert.equal(value.ok, false);
    assert.equal(value.partial, false);
    assert.deepEqual(value.changed, []);
    assert.match(result.stderr, /frontmatter conversion:/);
  }
});

test('plan is read-only; apply is idempotent and leaves frozen evidence untouched', async t => {
  const { root, home, paths } = await fixture(t);
  const evidence = join(home, 'examples/crate-puzzle/game-design-results/frozen.md');
  const inputs = join(home, 'examples/crate-puzzle/experiments/frozen-input.json');
  await mkdir(resolve(evidence, '..'), { recursive: true });
  await mkdir(resolve(inputs, '..'), { recursive: true });
  await writeFile(evidence, source, 'utf8');
  await writeFile(inputs, '{"unchanged":true}', 'utf8');
  const plan = JSON.parse(invoke(['--root', root, '--json']).stdout);
  assert.equal(plan.proposed, 3);
  assert.deepEqual(plan.changed, []);
  for (const path of paths) assert.equal(await readFile(path, 'utf8'), source);
  const applied = JSON.parse(invoke(['--root', root, '--apply']).stdout);
  assert.equal(applied.changed.length, 3);
  for (const path of paths) assert.equal(await readFile(path, 'utf8'), '---\nname: "fixture"\nstatus: "accepted"\n---\nOriginal body.\n');
  assert.equal(await readFile(evidence, 'utf8'), source);
  assert.equal(await readFile(inputs, 'utf8'), '{"unchanged":true}');
  assert.equal(JSON.parse(invoke(['--root', root, '--apply']).stdout).proposed, 0);
});

test('invalid later header refuses entire plan before writing any file', async t => {
  const { root, paths } = await fixture(t);
  await writeFile(paths[2], '---\n{"duplicate":1,"duplicate":2}\n---\nBody\n', 'utf8');
  const value = JSON.parse(invoke(['--root', root, '--apply'], 2).stdout);
  assert.equal(value.partial, false);
  assert.deepEqual(value.changed, []);
  assert.equal(await readFile(paths[0], 'utf8'), source);
});

test('shipped live records and templates no longer need flow conversion', () => {
  const result = JSON.parse(invoke(['--json']).stdout);
  assert.equal(result.proposed, 0);
  assert.deepEqual(result.changed, []);
});

test('linked collection is refused without changing source', async t => {
  const { root, home, paths } = await fixture(t);
  await symlink(join(home, 'templates/challenges'), join(home, 'templates/linked'), process.platform === 'win32' ? 'junction' : 'dir');
  const value = JSON.parse(invoke(['--root', root, '--apply'], 2).stdout);
  assert.equal(value.partial, false);
  assert.match(value.error.message, /linked collection/);
  assert.equal(await readFile(paths[0], 'utf8'), source);
});
