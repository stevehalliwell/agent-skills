// Verify missing-build recovery from a clean, self-contained skill copy.
// Usage: node tests/core/install-check.mjs (installed CJIT, Zig and Bash required).
// No downloads. Retains unique owned temp installation (path with spaces/Unicode)
// and workspace/report under build/install-check. Does not alter live installs.
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { cpSync, existsSync, mkdirSync, mkdtempSync, readFileSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const skill = resolve(root, '..');
mkdirSync(join(root, 'build/install-check'), { recursive: true });
const report = { ok: false, platform: process.platform, run: mkdtempSync(join(root, 'build/install-check/run-')), installation: mkdtempSync(join(tmpdir(), 'attendant native install café ')), cases: [] };
try {
  const copiedSkill = join(report.installation, 'attendant');
  cpSync(skill, copiedSkill, { recursive: true, filter: path => path !== join(root, 'build') });
  const native = join(copiedSkill, 'native');
  const project = join(report.run, 'target workspace café Ω');
  cpSync(join(root, 'tests/fixtures/project'), project, { recursive: true });
  for (const removed of ['scripts/attendant.mjs', 'scripts/yaml.mjs', 'test/attendant.test.js', 'references/migration.md', 'references/native-commands.md'])
    assert(!existsSync(join(copiedSkill, removed)), `Retired runtime/guidance remains: ${removed}`);
  for (const compiler of ['cjit', 'zig']) {
    const executable = join(native, `build/attendant-${compiler}${process.platform === 'win32' ? '.exe' : ''}`);
    assert(!existsSync(executable));
    const absent = spawnSync(executable, ['schema'], { cwd: project, encoding: 'utf8' });
    assert.equal(absent.error?.code, 'ENOENT');
    const build = spawnSync('bash', ['build.sh', compiler], { cwd: native, encoding: 'utf8', timeout: 180000 });
    writeFileSync(join(report.run, `build-${compiler}.log`), `${build.stdout || ''}\n${build.stderr || ''}`);
    assert.equal(build.status, 0, `Clean ${compiler} build: ${build.error || build.stderr}`);
    const help = spawnSync(executable, ['--help'], { cwd: project, encoding: 'utf8', timeout: 10000 });
    assert.equal(help.status, 0, help.error || help.stderr);
    assert.match(JSON.parse(help.stdout).usage, /doctor\|validate\|add-table\|create\|update\|sync\|schema\|query\|search/);
    const schema = spawnSync(executable, ['schema'], { cwd: project, encoding: 'utf8', timeout: 30000 });
    assert.equal(schema.status, 0, schema.error || schema.stderr);
    assert.deepEqual(JSON.parse(schema.stdout).collections.map(collection => collection.name), ['decisions', 'tasks']);
    assert.equal(readFileSync(join(project, '.pi/attendant.tables'), 'utf8'), readFileSync(join(root, 'tests/fixtures/project/.pi/attendant.tables'), 'utf8'));
    report.cases.push({ compiler, name: 'missing executable, isolated build from skill copy, help and return to original workspace' });
  }
  report.ok = true;
} finally { writeFileSync(join(root, 'build/install-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify({ ok: report.ok, cases: report.cases.length, run: report.run, installation: report.installation }));
