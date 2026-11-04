// Real macOS GLFW/Unix-socket control acceptance. Owned fixtures only.
// Usage: node tests/gui/control-macos-check.mjs after both GUI builds + --check.
import assert from 'node:assert/strict';
import { spawn, spawnSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { EventEmitter, once } from 'node:events';
import { chmodSync, cpSync, existsSync, mkdirSync, mkdtempSync, readFileSync, readdirSync, symlinkSync, unlinkSync, writeFileSync } from 'node:fs';
import net from 'node:net';
import { userInfo } from 'node:os';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { guiDirectory, verifyBuild } from '../../tools/gui-build-manifest.mjs';
assert.equal(process.platform, 'darwin', 'This suite tests macOS transport; use check.mjs for shared acceptance');
const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const environment = { ...process.env };
delete environment.DYLD_LIBRARY_PATH;
const gui = (compiler, testing = false) => join(root, guiDirectory, `attendant-gui-${compiler}${testing ? '-check' : ''}`);
const assets = join(root, 'build/deps/richmd/assets');
const hash = bytes => createHash('sha256').update(bytes).digest('hex');
const sleep = milliseconds => new Promise(resolveWait => setTimeout(resolveWait, milliseconds));
mkdirSync(join(root, 'build/control-check'), { recursive: true });
const report = { ok: false, platform: process.platform, architecture: process.arch, run: mkdtempSync(join(root, 'build/control-check/run-')), cases: [] };
function inventory(folder) {
  return Object.fromEntries(readdirSync(folder, { withFileTypes: true }).flatMap(entry => {
    const path = join(folder, entry.name);
    return entry.isDirectory() ? Object.entries(inventory(path)) : [[path, hash(readFileSync(path))]];
  }));
}
function fixture(name) {
  const project = join(report.run, name);
  cpSync(join(root, 'tests/fixtures/project'), project, { recursive: true });
  writeFileSync(join(project, '.gui-test-owned'), 'Owned native control fixture.\n');
  writeFileSync(join(project, 'README.md'), Array.from({ length: 180 }, (_, i) => `Line ${i + 1}: caf\u00e9 \u03a9 \ud83d\ude00 saved source.\n`).join(''));
  writeFileSync(join(project, 'other.md'), Array.from({ length: 180 }, (_, i) => `Other document line ${i + 1}.\n`).join(''));
  return project;
}
function control(compiler, project, args, status = 0, extraEnvironment = {}) {
  const child = spawnSync(gui(compiler, Boolean(extraEnvironment.ATTENDANT_GUI_TEST_REVEAL_HASH)), ['--project', project, ...args], {
    encoding: 'utf8', env: { ...environment, ...extraEnvironment }, timeout: 10000,
  });
  assert.equal(child.status, status, `${child.error || ''}\n${child.stderr}\n${child.stdout}`);
  return JSON.parse(child.stdout);
}
async function controlAsync(compiler, project, args, status = 0) {
  const child = spawn(gui(compiler), ['--project', project, ...args], { env: environment, stdio: ['ignore', 'pipe', 'pipe'] });
  let output = '', errors = '';
  child.stdout.on('data', bytes => { output += bytes; }); child.stderr.on('data', bytes => { errors += bytes; });
  const timer = setTimeout(() => child.kill(), 10000);
  try {
    const [code] = await once(child, 'close');
    assert.equal(code, status, `${errors}\n${output}`); return JSON.parse(output);
  } finally { clearTimeout(timer); if (child.exitCode === null) child.kill(); }
}
function viewer(compiler, project, mode, frames = 120) {
  const events = new EventEmitter(), records = [];
  const child = spawn(gui(compiler, true), ['--project', project, '--assets', assets, '--file', 'README.md', '--frames', String(frames), '--exercise-reveal', mode], { env: environment, stdio: ['ignore', 'pipe', 'pipe'] });
  let output = '', errors = '', ended = false;
  child.stdout.on('data', bytes => {
    output += bytes;
    for (;;) {
      const end = output.indexOf('\n'); if (end < 0) break;
      const line = output.slice(0, end); output = output.slice(end + 1);
      try { const value = JSON.parse(line); records.push(value); events.emit('record', value); } catch { errors += line + '\n'; }
    }
  });
  child.stderr.on('data', bytes => { errors += bytes; });
  const done = new Promise((resolveDone, reject) => {
    child.once('error', reject);
    child.once('close', code => { ended = true; events.emit('ended'); code === 0 ? resolveDone(records) : reject(new Error(`Viewer ${compiler}/${mode} exited ${code}: ${errors}`)); });
  });
  done.catch(() => {});
  const timer = setTimeout(() => child.kill(), 20000);
  return {
    child, done, records,
    event: name => new Promise((resolveEvent, reject) => {
      const existing = records.find(record => record.event === name);
      if (existing) { resolveEvent(existing); return; }
      if (ended) { reject(new Error(`Viewer ended before ${name}: ${errors}`)); return; }
      const onRecord = record => { if (record.event === name) { cleanup(); resolveEvent(record); } };
      const onEnd = () => { cleanup(); reject(new Error(`Viewer ended before ${name}: ${errors}`)); };
      const cleanup = () => { events.off('record', onRecord); events.off('ended', onEnd); };
      events.on('record', onRecord); events.on('ended', onEnd);
    }),
    stop: async () => { clearTimeout(timer); if (!ended) child.kill(); await done.catch(() => {}); },
  };
}
// Native local ABI: uint32 operation; reveal { version,line,project,path,hash }.
const PATH_BYTES = 16384, REVEAL_BYTES = (8 + 2 * PATH_BYTES + 65 + 3) & ~3;
const QUERY = 0x41545131, MATCH = 0x41544d31;
function queryPacket(project, operation = QUERY) {
  const packet = Buffer.alloc(4 + REVEAL_BYTES);
  packet.writeUInt32LE(operation, 0); packet.writeUInt32LE(1, 4);
  packet.write(project, 12, PATH_BYTES - 1, 'utf8'); return packet;
}
async function rawRequest(endpoint, payload, { fragmented = false, advertised = payload.length } = {}) {
  const socket = net.createConnection(endpoint);
  const chunks = []; socket.on('data', bytes => chunks.push(bytes));
  const timeout = setTimeout(() => socket.destroy(new Error('Raw client exceeded bound')), 5000);
  try {
    const closed = once(socket, 'close');
    await once(socket, 'connect');
    const header = Buffer.alloc(4); header.writeUInt32LE(advertised);
    if (fragmented) {
      socket.write(header.subarray(0, 2)); await sleep(10);
      socket.write(Buffer.concat([header.subarray(2), payload.subarray(0, 17)])); await sleep(10);
      socket.write(payload.subarray(17));
    } else socket.write(Buffer.concat([header, payload]));
    await closed; return Buffer.concat(chunks);
  } finally { clearTimeout(timeout); socket.destroy(); }
}
try {
  for (const compiler of ['cjit', 'zig']) {
    verifyBuild(compiler); verifyBuild(compiler, true);
    const other = compiler === 'cjit' ? 'zig' : 'cjit';
    const missing = join(report.run, `${compiler}-not-running`); mkdirSync(missing);
    assert.equal(control(other, missing, ['--inspect'], 1).code, 'gui-not-running');
    report.cases.push({ compiler, name: 'control reports absent viewer without starting GUI' });
    for (const mode of ['clean', 'dirty', 'stale', 'other']) {
      const project = fixture(`${compiler} ${mode} caf\u00e9 \u03a9`), before = inventory(project);
      const running = viewer(compiler, project, mode);
      try {
        await running.event('ready');
        const inspected = control(other, project, ['--inspect']);
        assert.equal(inspected.document.dirty, mode === 'dirty');
        assert.equal(inspected.selection.status, 'exact'); assert.equal(inspected.selection.view, 'source');
        assert.match(inspected.selection.text, /caf\u00e9 \u03a9 \ud83d\ude00/);
        assert.equal(inspected.selection.range_start, 0);
        assert.equal(inspected.selection.range_end, Array.from(inspected.selection.text).length);
        const target = mode === 'other' ? 'other.md' : 'README.md';
        const hint = control(other, project, ['--reveal', target, '--line', '80'], 0, mode === 'stale' ? { ATTENDANT_GUI_TEST_REVEAL_HASH: '0'.repeat(64) } : {});
        assert.deepEqual(hint, { ok: true, hint_updated: true, persistent: false, focus_changed: false });
        await running.event('inspected'); await running.event('received');
        const completed = await running.event('complete');
        assert(completed.checks > 0); assert.equal(completed.dirty, mode === 'dirty');
        assert.equal(completed.markerDraws > 0, mode === 'clean' || mode === 'other');
        await running.done;
        assert.deepEqual(inventory(project), before);
        assert(!existsSync(`/tmp/proto-pipes-${userInfo().uid}/attendant-gui-${running.child.pid}`));
        report.cases.push({ compiler, name: `${mode}: cross-compiler inspection/reveal, saved-hash Go guard, caret/selection/scroll/focus preservation and no source writes`, checks: completed.checks });
      } finally { await running.stop(); }
    }
    {
      const project = fixture(`${compiler} rejected controls`), before = inventory(project);
      const running = viewer(compiler, project, 'idle', 240);
      try {
        await running.event('ready');
        const sibling = fixture(`${compiler} rejected controls-other`);
        assert.equal(control(other, sibling, ['--inspect'], 1).code, 'gui-not-running');
        assert.equal(control(other, project, ['--reveal', join(sibling, 'README.md')], 1).code, 'reveal-not-allowed');
        assert.equal(control(other, project, ['--reveal', 'README.md', '--line', '999999'], 1).code, 'unmapped-line');
        assert.equal(control(other, project, ['--reveal', 'README.md', '--line', '0'], 2).code, 'usage');
        writeFileSync(join(project, 'bare-cr.md'), 'a\rb');
        assert.equal(control(other, project, ['--reveal', 'bare-cr.md'], 1).code, 'unmapped-line');
        symlinkSync(join(project, 'README.md'), join(project, 'alias.md'));
        assert.equal(control(other, project, ['--reveal', 'alias.md'], 1).code, 'unsupported-path');
        const second = viewer(other, project, 'idle', 90);
        try {
          await second.event('ready');
          assert.equal(control(other, project, ['--inspect'], 1).code, 'gui-ambiguous');
          assert.equal(control(other, project, ['--reveal', 'README.md'], 1).code, 'gui-ambiguous');
          await second.done;
        } finally { await second.stop(); }
        await running.done;
        const after = inventory(project); delete after[join(project, 'bare-cr.md')]; delete after[join(project, 'alias.md')];
        assert.deepEqual(after, before);
        report.cases.push({ compiler, name: 'unrelated/ambiguous viewers, sibling-path denial, line bounds, bare CR and application symlinks rejected without hint or source writes' });
      } finally { await running.stop(); }
    }
    {
      const project = fixture(`${compiler} endpoint policy`), before = inventory(project);
      const running = viewer(compiler, project, 'idle', 150);
      const endpoint = `/tmp/proto-pipes-${userInfo().uid}/attendant-gui-${process.pid}`;
      let fake;
      try {
        await running.event('ready'); assert(!existsSync(endpoint));
        fake = net.createServer(socket => socket.once('data', () => {
          const reply = Buffer.alloc(8); reply.writeUInt32LE(4); reply.writeUInt32LE(0x41545031, 4); socket.end(reply);
        }));
        fake.listen(endpoint); await once(fake, 'listening'); chmodSync(endpoint, 0o600);
        assert.equal((await controlAsync(other, project, ['--inspect'], 1)).code, 'gui-unavailable');
        chmodSync(endpoint, 0o666);
        assert.equal(control(other, project, ['--inspect'], 1).code, 'gui-unavailable');
        await new Promise((resolveClosed, reject) => fake.close(error => error ? reject(error) : resolveClosed())); fake = null;
        symlinkSync(`/tmp/proto-pipes-${userInfo().uid}/attendant-gui-${running.child.pid}`, endpoint);
        try { assert.equal(control(other, project, ['--inspect'], 1).code, 'gui-unavailable'); }
        finally { unlinkSync(endpoint); }
        assert.equal(control(other, project, ['--inspect']).document.dirty, false);
        await running.done; assert.deepEqual(inventory(project), before);
        report.cases.push({ compiler, name: 'unexpected discovery status, unsafe socket permissions and symlink endpoints fail conservatively; valid viewer remains usable' });
      } finally { if (fake) await new Promise(resolveClosed => fake.close(resolveClosed)); await running.stop(); }
    }
    {
      const project = fixture(`${compiler} framed transport`), before = inventory(project);
      const running = viewer(compiler, project, 'idle', 360);
      try {
        await running.event('ready');
        const endpoint = `/tmp/proto-pipes-${userInfo().uid}/attendant-gui-${running.child.pid}`;
        const reply = await rawRequest(endpoint, queryPacket(project), { fragmented: true });
        assert.equal(reply.length, 8); assert.equal(reply.readUInt32LE(0), 4); assert.equal(reply.readUInt32LE(4), MATCH);
        const unknown = await rawRequest(endpoint, queryPacket(project, 7));
        assert.equal(unknown.length, 8); assert.equal(unknown.readUInt32LE(4), 0);
        assert.equal((await rawRequest(endpoint, Buffer.from([1]))).length, 0);
        assert.equal((await rawRequest(endpoint, Buffer.alloc(0), { advertised: 4 + REVEAL_BYTES + 1 })).length, 0);
        const silent = net.createConnection(endpoint); await once(silent, 'connect');
        const started = Date.now(); await once(silent, 'close');
        assert(Date.now() - started >= 1500 && Date.now() - started < 4000);
        assert.equal(control(other, project, ['--inspect']).document.dirty, false);
        await running.done; assert.deepEqual(inventory(project), before);
        report.cases.push({ compiler, name: 'fragmented framing, unknown/short/oversize requests, silent-client deadline and responsive reconnect' });
      } finally { await running.stop(); }
    }
  }
  report.ok = true;
} finally { writeFileSync(join(root, 'build/control-check-results.json'), JSON.stringify(report, null, 2) + '\n'); }
console.log(JSON.stringify({ ok: report.ok, cases: report.cases.length, run: report.run }));
