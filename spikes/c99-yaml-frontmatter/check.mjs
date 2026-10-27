import { readFileSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import assert from 'node:assert/strict';
import { parseDocument, isMap } from '../../skills/attendant/scripts/yaml.mjs';

// Runner-equivalent boundary and document logic, not the runner's schema validation.
function split(text) {
  const open = /^---(\r?\n)/.exec(text);
  if (!open) return null;
  const rest = text.slice(open[0].length), close = /^---\r?$/m.exec(rest);
  if (!close) return null;
  return { yaml: rest.slice(0, close.index), body: rest.slice(close.index + close[0].length), closing: close[0], nl: open[1] };
}
function parse(s) {
  const d = parseDocument(s, { uniqueKeys: true });
  return d.errors.length || !isMap(d.contents) ? null : d;
}
function native(exe, yaml, mode) {
  return spawnSync(exe, [mode], { input: yaml, encoding: 'utf8', maxBuffer: 1024 * 1024 });
}
function values(yaml) { return parse(yaml)?.toJS(); }
const real = readFileSync(new URL('./fixtures/real.md', import.meta.url), 'utf8');
const synthetic = '---\n# heading café\nstatus: "old" # retained?\nflag: true\nempty: null\ncount: 42\ndate: 2024-02-29\ntime: 2024-02-29T12:30:00Z\nitems: [one, "two"]\nquoted: "true"\n---\n# body Ω\n';
const simple = '---\n# untouched\nstatus: old\nother: text\n---\nbody Ω\n';
const cases = { real, simple, simple_crlf: simple.replace(/\n/g, '\r\n'), lf: synthetic, crlf: synthetic.replace(/\n/g, '\r\n'), no_final_newline: synthetic.trimEnd(), duplicate: '---\na: 1\na: 2\n---\nbody', malformed: '---\na: [bad\n---\nbody', empty: '---\n---\nbody', missing: '# body', missing_close: '---\nstatus: old\nbody', nested: '---\na: {b: 1}\n---\nbody', alias: '---\nstatus: &s old\nother: *s\n---\nbody' };
const exes = process.argv.slice(2);
if (!exes.length) throw Error('usage: node check.mjs ./probe-cjit.exe ./probe-zig.exe');
for (const [name, text] of Object.entries(cases)) {
  const p = split(text), doc = p && parse(p.yaml);
  const validated = doc && doc.contents.items.every(({key, value}) => typeof key?.value === 'string' && key.value && (value == null || value.type === 'PLAIN' || value.constructor.name === 'Scalar' || (value.constructor.name === 'YAMLSeq' && value.items.every(x => x?.constructor.name === 'Scalar'))));
  console.log(`\n${name}: boundary=${!!p} runner-map=${!!doc} subset=${!!validated}`);
  if (name === 'lf') console.log(`  runner-values=${JSON.stringify(doc.toJS())}`);
  if (!p) continue;
  for (const exe of exes) {
    const result = native(exe, p.yaml, 'parse');
    console.log(`  ${exe} parse=${result.status === 0 ? 'accepted' : 'rejected'} ${result.stderr.trim()}`);
  }
  if (!doc || !Object.hasOwn(doc.toJS(), 'status') || name === 'alias') continue;
  const expected = doc.toJS(); expected.status = 'updated'; expected.spike_added = 'yes';
  for (const exe of exes) {
    const result = native(exe, p.yaml, 'edit');
    if (result.status !== 0) { console.log(`  ${exe} edit=failed ${result.stderr.trim()}`); continue; }
    const out = result.stdout.replace(/\r\n/g, '\n');
    const rebuilt = `---${p.nl}${out.replace(/\n/g, p.nl)}---${p.body}`;
    console.log(`  ${exe} edit semantic=${JSON.stringify(values(out)) === JSON.stringify(expected)} comments=${!p.yaml.includes('#') || out.includes('#')} unrelated-format=${out.replace(/status:.*\n/, '').replace(/spike_added:.*\n/, '') === p.yaml.replace(/status:.*\r?\n/, '').replace(/\r\n/g, '\n')} newline=${!rebuilt.replaceAll(p.nl, '').includes('\n')} body-bytes=${Buffer.from(split(rebuilt).body).equals(Buffer.from(p.body))}`);
  }
  doc.set('status', 'updated'); doc.set('spike_added', 'yes');
  const runner = doc.toString({lineWidth: 0}).replace(/\n/g, p.nl);
  const runnerText = `---${p.nl}${runner}---${p.body}`;
  console.log(`  runner edit semantic=${JSON.stringify(values(runner)) === JSON.stringify(expected)} comments=${!p.yaml.includes('#') || runner.includes('#')} unrelated-format=${runner.replace(/status:.*\r?\n/, '').replace(/spike_added:.*\r?\n/, '') === p.yaml.replace(/status:.*\r?\n/, '')} front-newline=${!runner.replaceAll(p.nl, '').includes('\n')} whole-newline=${!runnerText.replaceAll(p.nl, '').includes('\n')} body-bytes=${Buffer.from(p.body).equals(Buffer.from(split(runnerText).body))}`);
}
// A deliberately narrow source-splice experiment: reject anything but an isolated plain status line.
function splice(text) {
  const p = split(text); if (!p || !parse(p.yaml)) return null;
  if (/^[^\n]*[&*!|>{}\[\]]/m.test(p.yaml)) return null;
  const lines = p.yaml.split(p.nl), matches = lines.filter(x => /^status: [A-Za-z0-9_-]+$/.test(x));
  if (matches.length !== 1 || /^spike_added:/m.test(p.yaml)) return null;
  const changed = p.yaml.replace(/^status: [A-Za-z0-9_-]+$/m, 'status: updated') + `spike_added: "yes"${p.nl}`;
  const before = parse(p.yaml)?.toJS(), after = parse(changed)?.toJS();
  if (!before || !after) return null;
  before.status = 'updated'; before.spike_added = 'yes';
  if (JSON.stringify(before) !== JSON.stringify(after)) return null;
  return `---${p.nl}${changed}${p.closing}${p.body}`;
}
for (const [name, text] of Object.entries(cases)) {
  const out = splice(text);
  const expected = out && text.replace('status: old', 'status: updated').replace(`${split(text).nl}---`, `${split(text).nl}spike_added: "yes"${split(text).nl}---`);
  console.log(`splice ${name}: ${out ? 'accepted exact-expected=' + (out === expected) + ' body-bytes=' + Buffer.from(split(out).body).equals(Buffer.from(split(text).body)) : 'rejected'}`);
  if (out) assert.equal(split(out).body, split(text).body);
}
