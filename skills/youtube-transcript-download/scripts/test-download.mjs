#!/usr/bin/env node
/**
 * Offline CLI regression tests. Run: node scripts/test-download.mjs
 * A temporary fake transcript package tests output safety and errors without
 * network access, credentials, installation, or changes to local dependencies.
 */
import assert from "node:assert/strict";
import { copyFile, mkdir, mkdtemp, readFile, rm, writeFile } from "node:fs/promises";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { spawnSync } from "node:child_process";
import { fileURLToPath } from "node:url";

const directory = await mkdtemp(join(tmpdir(), "transcript-cli-test-"));
try {
  const script = join(directory, "download.mjs");
  await copyFile(fileURLToPath(new URL("./download.mjs", import.meta.url)), script);
  const dependency = join(directory, "node_modules", "youtube-transcript");
  await mkdir(dependency, { recursive: true });
  await writeFile(join(dependency, "package.json"), JSON.stringify({ type: "module", exports: "./index.mjs" }));
  await writeFile(join(dependency, "index.mjs"), `
export async function fetchTranscript(id, options) {
  if (id === "emptyempty1") return [];
  if (id === "errorerror1") throw new Error("Captions are unavailable.");
  return [{ text: "Caption text", offset: 1000, duration: 2000, lang: options?.lang === "fr" ? "en" : (options?.lang || "en") }];
}
`);
  const run = (...args) => spawnSync(process.execPath, [script, ...args], { encoding: "utf8", cwd: directory });
  const text = run("abcdefghijk");
  assert.equal(text.status, 0, text.stderr);
  assert.equal(text.stdout.trim(), "[0:01] Caption text");
  const json = run("https://youtu.be/abcdefghijk", "--format", "json", "--lang", "en");
  assert.equal(json.status, 0, json.stderr);
  assert.equal(JSON.parse(json.stdout).language, "en");
  assert.equal(JSON.parse(json.stdout).videoId, "abcdefghijk");
  const output = join(directory, "transcript.txt");
  assert.equal(run("abcdefghijk", "--output", output).status, 0);
  const original = await readFile(output, "utf8");
  const duplicate = run("abcdefghijk", "--output", output);
  assert.equal(duplicate.status, 1);
  assert.match(duplicate.stderr, /Refusing to overwrite existing output/);
  assert.equal(await readFile(output, "utf8"), original);
  const missing = run("abcdefghijk", "--output", join(directory, "missing", "transcript.txt"));
  assert.equal(missing.status, 1);
  assert.match(missing.stderr, /Output directory does not exist/);
  for (const [id, message] of [["emptyempty1", /No transcript available/], ["errorerror1", /Captions are unavailable/]]) {
    const failure = run(id);
    assert.equal(failure.status, 1);
    assert.match(failure.stderr, message);
    assert.doesNotMatch(failure.stderr, /at file:/);
    assert.equal(failure.stdout, "");
  }
  const language = run("abcdefghijk", "--lang", "fr");
  assert.equal(language.status, 1);
  assert.match(language.stderr, /Requested language fr, but received en/);
  assert.equal(run("invalid").status, 1);
  assert.equal(run("abcdefghijk", "--format", "invalid").status, 1);
  assert.equal(run("--help").status, 0);
  console.log("transcript CLI offline regression tests: OK");
} finally {
  await rm(directory, { recursive: true, force: true });
}
