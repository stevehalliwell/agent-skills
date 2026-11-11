// Shared GUI acceptance coordinator. Behavior assertions live in generated C.
// Usage: node tests/gui/check.mjs (build both compilers, production and --check first).
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { mkdirSync, mkdtempSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { verifyBuild } from '../../tools/gui-build-manifest.mjs';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const guiEnvironment = { ...process.env };
if (process.platform === 'darwin') delete guiEnvironment.DYLD_LIBRARY_PATH;
mkdirSync(join(root, 'build/gui-check'), { recursive: true });
const report = { ok: false, platform: process.platform, architecture: process.arch, run: mkdtempSync(join(root, 'build/gui-check/run-')), cases: [] };
function nativeChecks(compiler) {
  const built = spawnSync('bash', ['tools/build-gui-tests.sh', compiler], { cwd: root, env: guiEnvironment, encoding: 'utf8', timeout: 180000 });
  assert.equal(built.status, 0, `${built.error || ''}\n${built.stderr}\n${built.stdout}`);
  const runner = built.stdout.trim(), directory = join(report.run, `native-${compiler}`);
  mkdirSync(directory);
  const result = spawnSync(runner, ['-tsv'], { cwd: root, encoding: 'utf8', timeout: 900000,
    env: { ...guiEnvironment, ATTENDANT_TEST_DIRECTORY: directory, ATTENDANT_TEST_COMPILER: compiler } });
  writeFileSync(join(directory, 'results.tsv'), result.stdout || '');
  writeFileSync(join(directory, 'diagnostics.log'), result.stderr || '');
  report.native ??= [];
  report.native.push({ compiler, runner, directory, status: result.status });
  assert.equal(result.status, 0, `${result.error || ''}\n${result.stderr}\n${result.stdout}`);
  const rows = result.stdout.trimEnd().split('\n').map(line => line.replace(/\r$/, '').split('\t'));
  assert.deepEqual(rows[0], ['kind', 'status', 'source', 'name']);
  assert.deepEqual(rows[1], ['suite', '10', '', '']);
  assert.deepEqual(rows.at(-1), ['complete', '0', '10', '10']);
  assert.equal(rows.length, 13);
  const cases = rows.slice(2, -1);
  assert(cases.every(row => row.length === 4 && row[0] === 'test' && row[1] === 'ok'));
  const finite = ['production_render_abi_screenshot_without_workspace_writes', 'existing_c_document_selection_ipc_model',
    'terminal_render_failure_preserves_source', 'partial_discovery_diagnostics_preserve_source',
    'reference_navigation_and_tab_reuse_without_writes', 'literal_path_undo_isolation_and_redo',
    'real_edit_feedback_and_explicit_resolution', 'failed_queue_keeps_save_and_other_files'];
  const previews = ['preview_multi_cross_compiler_inspection_context_save_send_and_resolution',
    'preview_context_cross_compiler_inspection_context_save_send_and_resolution'];
  const expected = [...finite.map(name => ['tests/gui/integration/checks.c', name]),
    ...previews.map(name => ['tests/gui/integration/preview.c', name])];
  assert.deepEqual(cases.map(row => row.slice(2)).sort(), expected.sort());
  const models = [...result.stderr.matchAll(/^model-checks:(\d+)\r?$/gm)];
  assert.equal(models.length, 1);
  const checks = Number(models[0][1]); assert(Number.isInteger(checks) && checks > 0 && checks <= 2147483647);
  for (const row of cases) report.cases.push({ compiler, name: row[3], source: row[2],
    ...(row[3] === 'existing_c_document_selection_ipc_model' ? { result: { ok: true, checks } } : {}) });
}
try {
  // Validate opposite-compiler binary too before any cross-compiler inspection.
  for (const compiler of ['cjit', 'zig']) { verifyBuild(compiler); verifyBuild(compiler, true); }
  for (const compiler of ['cjit', 'zig']) nativeChecks(compiler);
  report.ok = true;
} finally { writeFileSync(join(root, 'build/gui-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify({ ok: report.ok, cases: report.cases.length, run: report.run }));
