import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import test from "node:test";
import { fileURLToPath } from "node:url";

const references = path.dirname(fileURLToPath(import.meta.url));
const metricsScript = path.join(references, "style-profile-metrics.mjs");
const compareScript = path.join(references, "style-profile-compare.mjs");
const metric = (...files) => JSON.parse(execFileSync(process.execPath, [metricsScript, ...files], { encoding: "utf8" }));

test("filters conjunctions and conversion debris from vocabulary while retaining POS evidence", (t) => {
  const directory = fs.mkdtempSync(path.join(os.tmpdir(), "style-profile-metrics-"));
  t.after(() => fs.rmSync(directory, { recursive: true, force: true }));
  const file = path.join(directory, "sample.md");
  const secondFile = path.join(directory, "second.md");
  fs.writeFileSync(file, "And I logged 2026 e-mail for the authors to write clear prose.\n\nBut authors write clear prose for their local readers now.\n");
  fs.writeFileSync(secondFile, "Nouns shape an article.\n");

  const report = metric(file);
  const vocabulary = report.metrics.vocabulary.mostCommonWords.map(({ word }) => word);
  assert.equal(report.schemaVersion, 6);
  assert.equal(typeof report.metrics.vocabulary.windowedUniqueWordRatio, "number");
  assert.equal(report.metrics.comparisonDistributions.listItemsPer1000Words.count, 1);
  assert.deepEqual(vocabulary.includes("and"), false);
  assert.deepEqual(vocabulary.includes("but"), false);
  assert.deepEqual(vocabulary.includes("i"), false);
  assert.deepEqual(vocabulary.includes("e"), false);
  assert.deepEqual(vocabulary.includes("2026"), false);
  assert.deepEqual(vocabulary.includes("for"), false);
  assert.deepEqual(vocabulary.includes("the"), false);
  assert.deepEqual(vocabulary.includes("to"), false);
  assert.deepEqual(vocabulary.includes("mail"), true);

  const wordTypes = report.metrics.wordTypes;
  assert.equal(wordTypes.tagSet, "Universal POS");
  assert.equal(wordTypes.sentencePercentages.NOUN.count, 2);
  assert.equal(wordTypes.paragraphPercentages.NOUN.count, 2);
  assert.equal(wordTypes.article.counts.CCONJ, 2);
  assert.equal(wordTypes.articlePercentages.NOUN.count, 1);
  assert.ok(Math.abs(Object.values(wordTypes.article.percentages).reduce((sum, value) => sum + value, 0) - 100) < 0.01);
  assert.equal(metric(file, secondFile).metrics.wordTypes.articlePercentages.NOUN.count, 2);
});

test("separates short standalone labels from paragraph cadence", (t) => {
  const directory = fs.mkdtempSync(path.join(os.tmpdir(), "style-profile-labels-"));
  t.after(() => fs.rmSync(directory, { recursive: true, force: true }));
  const file = path.join(directory, "sample.md");
  fs.writeFileSync(file, "A label here.\n\nThis substantive paragraph has enough words to remain in cadence analysis.\n\n# Markdown heading\n");

  const report = metric(file).metrics;
  assert.equal(report.standaloneLabelsPer1000Words > 0, true);
  assert.equal(report.standaloneLabelLengthWords.median, 3);
  assert.equal(report.paragraphLengthWords.count, 1);
  assert.equal(report.paragraphLengthWords.median, 11);
});

test("compares POS word-type metrics", (t) => {
  const directory = fs.mkdtempSync(path.join(os.tmpdir(), "style-profile-compare-"));
  t.after(() => fs.rmSync(directory, { recursive: true, force: true }));
  const corpus = path.join(directory, "corpus.md");
  const target = path.join(directory, "target.md");
  const profile = path.join(directory, "profile.metrics.json");
  fs.writeFileSync(corpus, "Writers choose precise verbs.\n");
  fs.writeFileSync(target, "I choose verbs.\n");
  fs.writeFileSync(profile, JSON.stringify(metric(corpus)));

  const comparison = JSON.parse(execFileSync(process.execPath, [compareScript, profile, target], { encoding: "utf8" }));
  assert.equal(comparison.comparisons.length, 1);
  assert.equal(comparison.scoring.calibration.cadenceAndStructureWeight, 0.45);
  assert.equal(comparison.scoring.calibration.voiceAndPunctuationWeight, 0.35);
  assert.equal(comparison.comparisons[0].metricComparisons.some(({ metric: name }) => name === "word type PRON, article percentage"), true);
  assert.equal(comparison.comparisons[0].metricComparisons.some(({ metric: name }) => name.includes("sentence percentage median")), false);
  assert.equal(comparison.comparisons[0].groupScores.find(({ group }) => group === "lexicon").metricCount, 2);
});

test("weights central paragraph cadence and does not penalise absent sparse punctuation", (t) => {
  const directory = fs.mkdtempSync(path.join(os.tmpdir(), "style-profile-compare-"));
  t.after(() => fs.rmSync(directory, { recursive: true, force: true }));
  const sentence = "word word word word word word word word word word.";
  const write = (name, count) => {
    const file = path.join(directory, `${name}.md`);
    fs.writeFileSync(file, Array.from({ length: count }, () => sentence).join(" "));
    return file;
  };
  const low = write("low", 2);
  const centre = write("centre", 4);
  const high = write("high", 6);
  const profile = path.join(directory, "profile.metrics.json");
  fs.writeFileSync(profile, JSON.stringify(metric(low, centre, high)));
  fs.writeFileSync(profile.replace(".metrics.json", ".md"), `# Style profile: test

## Verification calibration
- Cadence and structure weight: 50%
- Voice and punctuation weight: 25%
- Lexicon weight: 15%
- POS composition weight: 10%
- Standalone-label maximum words: 7
`);

  const comparison = JSON.parse(execFileSync(process.execPath, [compareScript, profile, low, centre], { encoding: "utf8" }));
  const byTarget = Object.fromEntries(comparison.comparisons.map((result) => [result.target, result]));
  assert.ok(byTarget[centre].statisticalDeviationScore < byTarget[low].statisticalDeviationScore);
  assert.equal(byTarget[centre].groupScores.find(({ group }) => group === "cadence and structure").deviation, 0);
  assert.equal(byTarget[centre].metricComparisons.find(({ metric: name }) => name === "semicolon per 1,000 words").deviation, 0);
  assert.equal(comparison.scoring.calibration.cadenceAndStructureWeight, 0.5);
});
