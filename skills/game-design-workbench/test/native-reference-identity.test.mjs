// Q12: unresolved references remain declarations, not invented vertices or repaired files.
// GD_NATIVE_EXE selects native CLI. Fixtures are private temporary directories, never example edits.
import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, readFile, readdir, rm } from 'node:fs/promises';
import { join, resolve } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';

const executable = process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
const missing = 'm/not-yet-defined', unloaded = 'unloaded/not-yet-defined';
function invoke(args, status = 0) {
  const result = spawnSync(executable, [...args, '--json'], { encoding: 'utf8', timeout: 30000,
    maxBuffer: 16 * 1024 * 1024, windowsHide: true, env: { ...process.env, GD_AGENT: '1' } });
  assert.equal(result.status, status, result.stderr + '\n' + result.stdout);
  if (status === 0) assert.equal(result.stderr, '');
  const value = JSON.parse(result.stdout);
  assert.equal(value.ok, status === 0);
  return value;
}
function record(name, status, fields = '') {
  return `---\nname: ${name}\nstatus: ${status}\n${fields}---\nDeclaration only; no player observation.\n`;
}
async function fixture(t) {
  const root = await mkdtemp(join(tmpdir(), 'gd-reference-identity-'));
  t.after(() => rm(root, { recursive: true, force: true }));
  for (const folder of ['m', 'd', 'a', 'decoys', 'unloaded']) await mkdir(join(root, folder));
  await writeFile(join(root, 'm/known.md'), record('known', 'accepted'), 'utf8');
  await writeFile(join(root, 'decoys/not-yet-defined.md'), record('not-yet-defined', 'accepted'), 'utf8');
  const source = join(root, 'd/basis.md');
  await writeFile(source, record('basis', 'proposed', `mechanics:\n  - ${missing}\n  - ${unloaded}\n  - m/known\n  - ${missing}\n`), 'utf8');
  await writeFile(join(root, 'a/intention.md'), record('intention', 'proposed', 'dynamics:\n  - d/basis\n'), 'utf8');
  const bindings = ['--mechanics', `m=${root}/m`, '--dynamics', `d=${root}/d`,
    '--aesthetics', `a=${root}/a`, '--related', `decoys=${root}/decoys`];
  return { root, source, bindings };
}
async function snapshot(folder) {
  const result = [];
  for (const entry of (await readdir(folder, { withFileTypes: true })).sort((a, b) => a.name.localeCompare(b.name))) {
    const path = join(folder, entry.name);
    if (entry.isDirectory()) result.push(...await snapshot(path));
    else result.push([path, (await readFile(path)).toString('base64')]);
  }
  return result;
}
const gaps = bindings => invoke(['query', '--question', 'declaration-gaps', '--projection', 'mda', ...bindings]);

test('missing and unloaded aliases preserve exact duplicate declarations, source lines and negative selectors', { skip: !executable }, async t => {
  const { root, source, bindings } = await fixture(t), before = await snapshot(root);
  try {
    const domain = invoke(['mda', ...bindings]);
    assert.equal(domain.coverage.graph_references_complete, false);
    assert.equal(domain.coverage.selected_node_count, 3);
    assert.ok(!domain.data.nodes.some(node => node.reference === missing || node.reference === unloaded));
    assert.equal(domain.data.links.filter(link => link.reference === missing && link.resolution === 'missing' && link.target === null).length, 2);
    assert.equal(domain.data.links.filter(link => link.reference === unloaded && link.resolution === 'unloaded' && link.target === null).length, 1);
    assert.ok(domain.data.findings.some(row => row.code === 'mda.reference_missing' && row.reference === missing));
    const value = gaps(bindings), inventory = value.data.declaration_gap_inventory;
    assert.equal(value.coverage.query_complete, true);
    assert.equal(value.coverage.source_incomplete, true);
    assert.equal(value.coverage.final_outcomes_proven, false);
    assert.equal(inventory.inventory_checked, true);
    assert.equal(inventory.missing_metadata_checked, false);
    assert.deepEqual([inventory.rows, inventory.resolved, inventory.missing, inventory.unloaded, inventory.ambiguous], [5, 2, 2, 1, 0]);
    const lines = (await readFile(source, 'utf8')).split(/\r?\n/);
    for (const gap of inventory.gaps) {
      assert.equal(gap.source.reference, 'd/basis');
      assert.equal(resolve(gap.source.path), resolve(source));
      assert.equal(gap.source.status, 'proposed');
      assert.equal(gap.field, 'mechanics');
      assert.equal(lines[gap.line - 1].trim(), '- ' + gap.reference);
    }
    assert.equal(new Set(inventory.gaps.map(row => row.line)).size, 3);
    assert.equal(invoke(['mda', ...bindings, '--from', missing], 2).errors[0].code, 'mda.root');
    assert.equal(invoke(['query', '--question', 'direct-connections', '--projection', 'mda', ...bindings, '--from', missing], 2).errors[0].code, 'query.selector_missing');
  } finally { assert.deepEqual(await snapshot(root), before); }
});

test('only explicit alias loading or exact new definition changes reference resolution', { skip: !executable }, async t => {
  const { root, source, bindings } = await fixture(t), originalSource = await readFile(source);
  const initially = gaps(bindings).data.declaration_gap_inventory;
  assert.equal(initially.unloaded, 1);
  const loaded = gaps([...bindings, '--related', `unloaded=${root}/unloaded`]).data.declaration_gap_inventory;
  assert.deepEqual([loaded.missing, loaded.unloaded], [3, 0]);
  // Deliberate fixture authoring, not a native repair operation.
  await writeFile(join(root, 'm/not-yet-defined.md'), record('not-yet-defined', 'accepted'), 'utf8');
  const before = await snapshot(root);
  try {
    const domain = invoke(['mda', ...bindings]);
    assert.equal(domain.coverage.selected_node_count, 4);
    const declarations = domain.data.links.filter(link => link.reference === missing);
    assert.equal(declarations.length, 2);
    assert.ok(declarations.every(link => link.resolution === 'resolved' && link.target === missing && link.target_status === 'accepted'));
    assert.ok(domain.data.links.some(link => link.reference === unloaded && link.resolution === 'unloaded'));
    const after = gaps(bindings).data.declaration_gap_inventory;
    assert.deepEqual([after.rows, after.resolved, after.missing, after.unloaded], [5, 4, 0, 1]);
    assert.deepEqual(await readFile(source), originalSource);
  } finally { assert.deepEqual(await snapshot(root), before); }
});
