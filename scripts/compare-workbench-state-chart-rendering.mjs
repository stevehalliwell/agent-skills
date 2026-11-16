#!/usr/bin/env node
// Windows-only, explicitly authorized rendering diagnosis. No builds/source writes.
// One or two sequential 600-frame windows; 30-second guard never kills a viewer.
// Query: shipped Mario Shellback, ShellState + Life. Capture both desktop modes
// and final native framebuffer. Does not synthesize pointer/keyboard input.
import { spawn, execFile } from 'node:child_process';
import { promisify } from 'node:util';
import { mkdir, open, readFile, readdir, stat, writeFile } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { resolve, dirname, join, relative } from 'node:path';
import { fileURLToPath } from 'node:url';

const execute = promisify(execFile);
const repository = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const game = join(repository, 'skills/game-design-workbench/examples/little-plumber-guy-nes/game-design');
const options = {};
const results = [];
let output, current, created = false;
const sleep = milliseconds => new Promise(done => setTimeout(done, milliseconds));
function emit(value) {
  if (process.stdout.isTTY && !options.json && !process.env.GD_AGENT && !process.env.CI) {
    console.log(`${value.ok ? 'PASS' : 'FAIL'}: ${value.message ?? 'comparison captured; inspect images before accepting rendering'}`);
    if (value.output) console.log(value.output);
  } else console.log(JSON.stringify(value));
}
async function sourceHash(directory = game, hash = createHash('sha256')) {
  for (const entry of (await readdir(directory, { withFileTypes: true })).sort((a, b) => a.name.localeCompare(b.name))) {
    const path = join(directory, entry.name);
    if (entry.isDirectory()) await sourceHash(path, hash);
    else if (entry.isFile()) hash.update(relative(game, path)).update('\0').update(await readFile(path));
  }
  return hash;
}
async function viewer(arguments_) {
  try {
    const result = await execute(options.client, ['viewer', ...arguments_], { encoding: 'utf8', windowsHide: true, maxBuffer: 16 * 1024 * 1024 });
    return JSON.parse(result.stdout);
  } catch (error) {
    throw new Error(`Viewer ${arguments_[0]} failed; no mutation retry: ${error.stdout || error.message}`);
  }
}
try {
  for (let index = 2; index < process.argv.length; index++) {
    const name = process.argv[index];
    if (name === '--help') {
      console.log('Usage: node scripts/compare-workbench-state-chart-rendering.mjs [--baseline EXE] [--candidate EXE] --client EXE --output NEW_DIRECTORY --allow-windows [--json]\nRequire at least one build; each opens one 600-frame Mario window; stage exact Shellback joint hypotheses, capture both modes and native framebuffer; normal exit only. A stalled viewer remains running. No source writes or pointer/keyboard automation. Exit 0 means capture/byte parity/source checks passed, NOT human visual acceptance; failure exits2 with partial artifacts.');
      process.exit(0);
    }
    if (name === '--allow-windows' || name === '--json') options[name.slice(2)] = true;
    else if (['--baseline', '--candidate', '--client', '--output'].includes(name) && process.argv[index + 1] && !process.argv[index + 1].startsWith('--')) {
      if (options[name.slice(2)]) throw new Error(`Duplicate ${name}`);
      options[name.slice(2)] = resolve(process.argv[++index]);
    } else throw new Error(`Unknown/missing option: ${name}`);
  }
  if (!options['allow-windows']) throw new Error('Require explicit --allow-windows after human launch authorization.');
  for (const name of ['client', 'output']) if (!options[name]) throw new Error(`Require --${name}`);
  if (!options.baseline && !options.candidate) throw new Error('Require --baseline or --candidate.');
  if (process.platform !== 'win32') throw new Error('Windows desktop required.');
  for (const name of ['baseline', 'candidate', 'client'].filter(name => options[name])) if (!(await stat(options[name])).isFile()) throw new Error(`${name} must be an existing executable`);
  output = options.output;
  await mkdir(output); // Refuse existing output; never overwrite prior evidence.
  created = true;
  const before = (await sourceHash()).digest('hex');
  for (const name of ['baseline', 'candidate'].filter(name => options[name])) {
    const nativeImage = join(output, name + '-native.png');
    const arguments_ = ['view', 'explorer', '--mode', 'state-charts', '--frames', '600', '--screenshot', nativeImage];
    for (const collection of ['nouns', 'verbs', 'axes', 'mechanics', 'interactions', 'resources', 'control-vars', 'skill-atoms', 'challenges', 'dynamics', 'aesthetics']) arguments_.push('--' + collection, `${collection}=${join(game, collection)}`);
    arguments_.push('--related', `systems=${join(game, 'systems')}`);
    const stdout = await open(join(output, name + '.stdout.log'), 'wx');
    const stderr = await open(join(output, name + '.stderr.log'), 'wx');
    current = spawn(options[name], arguments_, { cwd: repository, stdio: ['ignore', stdout.fd, stderr.fd] });
    const result = { build: name, executable: options[name], pid: current.pid, captures: [], nativeImage };
    const closed = new Promise(done => { current.once('error', error => { result.spawnError = error.message; done({ code: null, signal: null }); }); current.once('close', (code, signal) => done({ code, signal })); });
    results.push(result);
    const deadline = Date.now() + 30000;
    try {
      let session;
      while (!session && !result.spawnError && Date.now() < deadline && current.exitCode === null) {
        session = (await viewer(['list', '--json'])).viewers.find(item => item.session.startsWith(current.pid + '-'))?.session;
        if (!session) await sleep(30); // Read-only discovery only; no mutation retry.
      }
      if (!session) throw new Error(result.spawnError ?? 'Own viewer session unavailable before stall guard/exit.');
      result.session = session;
      const shared = ['--target', session, '--projection', 'state-charts'];
      for (const [control, value] of [['object', 'nouns/shellback'], ['axis', 'axes/ShellState'], ['companion_axis', 'axes/Life']]) {
        const token = (await viewer(['controls', ...shared, '--form', 'chart'])).forms[0].control_token;
        await viewer(['configure', ...shared, '--form', 'chart', '--control', control, '--value', value, '--expect', token]);
      }
      await viewer(['action', ...shared, '--action', 'chart.run']);
      let retained;
      for (const mode of ['grid', 'statechart']) {
        const token = (await viewer(['controls', ...shared, '--form', 'presentation'])).forms[0].control_token;
        await viewer(['configure', ...shared, '--form', 'presentation', '--control', 'state_chart_mode', '--value', mode, '--expect', token]);
        const page = await viewer(['result', ...shared, '--form', 'query', '--fields', 'axis_definition,companion_axis_definition,state_values', '--offset', '0', '--limit', '32']);
        if (page.next_offset !== null) throw new Error('Selected result exceeds one bounded capture page.');
        if (retained && retained !== page.data) throw new Error('Mode change altered native retained query bytes.');
        retained = page.data;
        await writeFile(join(output, `${name}-${mode}-result.json`), Buffer.from(page.data, 'base64'));
        try {
          const capture = await execute('pwsh', ['-NoProfile', '-File', join(repository, 'scripts/capture-native-review-window.ps1'), '-ProcessId', String(current.pid), '-OutputDirectory', output, '-BringToFront'], { encoding: 'utf8', windowsHide: true });
          result.captures.push({ mode, ...JSON.parse(capture.stdout) });
        } catch (error) {
          // Keep native framebuffer diagnosis, but never count failed desktop capture as pass.
          result.captures.push({ mode, ok: false, code: 'desktop-capture', message: error.message });
        }
      }
      result.queryBytesUnchanged = true;
      const remaining = Math.max(0, deadline - Date.now());
      const guard = new Promise(done => setTimeout(() => done(null), remaining).unref());
      const exit = await Promise.race([closed, guard]);
      if (!exit) throw new Error(`30-second stall guard reached; viewer ${current.pid} left running.`);
      result.exit = exit;
      if (exit.code !== 0 || !(await stat(nativeImage)).isFile()) throw new Error('Viewer did not exit0 with native framebuffer image.');
    } finally { await stdout.close(); await stderr.close(); }
  }
  const after = (await sourceHash()).digest('hex');
  if (before !== after) throw new Error('Example source hash changed during diagnosis.');
  const desktopPassed = results.every(result => result.captures.every(capture => capture.ok));
  const report = { ok: desktopPassed, output, nativeCapturePassed: true, desktopCapturePassed: desktopPassed, sourceHash: before, examplesUnchanged: true, results, claim: 'Capture/byte parity only; images and human pointer acceptance require review.' };
  if (!desktopPassed) { report.code = 'state-chart.desktop-capture'; report.message = 'Native capture completed, desktop capture incomplete; inspect recorded errors.'; process.exitCode = 2; }
  await writeFile(join(output, 'comparison.json'), JSON.stringify(report, null, 2), 'utf8');
  emit(report);
} catch (error) {
  const report = { ok: false, code: 'state-chart.render-comparison', message: error.message, output, results, leftRunning: current?.exitCode === null ? current.pid : null };
  if (created) {
    try { await writeFile(join(output, 'failure.json'), JSON.stringify(report, null, 2), 'utf8'); }
    catch (artifactError) { report.artifact_error = artifactError.message; }
  }
  emit(report);process.exitCode = 2;
}
