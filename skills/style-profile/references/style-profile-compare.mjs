#!/usr/bin/env node
/**
 * Compare one or more Markdown targets with a style-profile metrics sidecar.
 *
 * Usage:
 *   style-profile-compare.mjs <profile.metrics.json> <Markdown files...>
 *
 * Scores are heuristic review signals, not conformance grades or rewrite
 * instructions. Correlated metrics are grouped before weighting so, for
 * example, POS tags cannot outweigh paragraph cadence merely by quantity.
 */

import { execFileSync } from "node:child_process";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const [profilePath, ...targets] = process.argv.slice(2);
if (!profilePath || targets.length === 0 || profilePath === "--help" || profilePath === "-h") {
  console.error("Usage: style-profile-compare.mjs <profile.metrics.json> <Markdown files...>");
  process.exit(profilePath === "--help" || profilePath === "-h" ? 0 : 2);
}

const profile = JSON.parse(fs.readFileSync(profilePath, "utf8"));
const numeric = (value) => typeof value === "number" && Number.isFinite(value);
if (profile.schemaVersion !== 6 || !profile.metrics?.comparisonDistributions || !profile.metrics?.measurement) {
  console.error(`${profilePath} is not a schema-version-6 style metrics sidecar; regenerate it with style-profile-metrics.mjs.`);
  process.exit(2);
}

const defaultCalibration = {
  cadenceAndStructureWeight: 0.45,
  voiceAndPunctuationWeight: 0.35,
  lexiconWeight: 0.1,
  posCompositionWeight: 0.1,
  standaloneLabelMaxWords: 7
};
const calibrationFromProfile = () => {
  const markdownPath = profilePath.replace(/\.metrics\.json$/u, ".md");
  if (!fs.existsSync(markdownPath)) return defaultCalibration;
  const markdown = fs.readFileSync(markdownPath, "utf8");
  const heading = markdown.match(/^## Verification calibration\s*$/mu);
  const section = heading ? markdown.slice((heading.index ?? 0) + heading[0].length).split(/^##\s/m)[0] : "";
  if (!section.trim()) return defaultCalibration;
  const value = (label, suffix = "%") => {
    const match = section.match(new RegExp(`^-\\s*${label}:\\s*([0-9]+(?:\\.[0-9]+)?)${suffix}\\s*$`, "mu"));
    return match ? Number(match[1]) : null;
  };
  const calibration = {
    cadenceAndStructureWeight: value("Cadence and structure weight") / 100,
    voiceAndPunctuationWeight: value("Voice and punctuation weight") / 100,
    lexiconWeight: value("Lexicon weight") / 100,
    posCompositionWeight: value("POS composition weight") / 100,
    standaloneLabelMaxWords: value("Standalone-label maximum words", "")
  };
  if (Object.values(calibration).some((item) => !numeric(item) || item < 0) || !Number.isInteger(calibration.standaloneLabelMaxWords) || calibration.standaloneLabelMaxWords < 1) {
    console.error(`${markdownPath} has an invalid Verification calibration section.`);
    process.exit(2);
  }
  return calibration;
};
const calibration = calibrationFromProfile();
if (profile.metrics.measurement.standaloneLabelMaxWords !== calibration.standaloneLabelMaxWords) {
  console.error(`${profilePath} was generated with standalone-label maximum words ${profile.metrics.measurement.standaloneLabelMaxWords}, but its profile declares ${calibration.standaloneLabelMaxWords}; regenerate the sidecar with --paragraph-label-max-words ${calibration.standaloneLabelMaxWords}.`);
  process.exit(2);
}

const metricsScript = path.join(path.dirname(fileURLToPath(import.meta.url)), "style-profile-metrics.mjs");
const metricTarget = (target) => JSON.parse(execFileSync(process.execPath, [metricsScript, "--paragraph-label-max-words", String(calibration.standaloneLabelMaxWords), target], { encoding: "utf8" })).metrics;
const rounded = (value) => Number(value.toFixed(3));
const addIfPresent = (findings, comparison) => { if (comparison) findings.push(comparison); };

// Distance from the corpus centre is retained within the IQR, at a reduced
// scale set by the robust half-IQR. This distinguishes a central fit from a
// merely non-outlying value without treating the median as a drafting quota.
const centeredDifference = ({ name, target, expected, minimumSpread = 0.01 }) => {
  if (![target, expected?.p25, expected?.median, expected?.p75].every(numeric)) return null;
  const spread = Math.max((expected.p75 - expected.p25) / 2, Math.abs(expected.median) * 0.1, minimumSpread);
  return { metric: name, target, expected, deviation: rounded(Math.abs(target - expected.median) / spread) };
};
const mean = (values) => values.length ? values.reduce((sum, value) => sum + value, 0) / values.length : 0;
const weightedMean = (groups) => {
  const available = groups.filter((group) => group.findings.length);
  const totalWeight = available.reduce((sum, group) => sum + group.weight, 0);
  return totalWeight ? available.reduce((sum, group) => sum + mean(group.findings.map((finding) => finding.deviation)) * group.weight, 0) / totalWeight : 0;
};

const compare = (target) => {
  const corpus = profile.metrics;
  const distributions = corpus.comparisonDistributions;
  const targetMetrics = metricTarget(target);
  const groups = [
    { name: "cadence and structure", weight: calibration.cadenceAndStructureWeight, findings: [] },
    { name: "voice and punctuation", weight: calibration.voiceAndPunctuationWeight, findings: [] },
    { name: "lexicon", weight: calibration.lexiconWeight, findings: [] },
    { name: "POS composition", weight: calibration.posCompositionWeight, findings: [] }
  ];
  const [cadence, voice, lexicon, pos] = groups;
  const add = (group, options) => addIfPresent(group.findings, centeredDifference(options));

  for (const [label, key, minimumSpread] of [
    ["sentence length median", "sentenceLengthWords", 1],
    ["paragraph words median", "paragraphLengthWords", 5],
    ["paragraph sentences median", "paragraphLengthSentences", 0.5]
  ]) add(cadence, { name: label, target: targetMetrics[key]?.median, expected: corpus[key], minimumSpread });
  add(cadence, { name: "short sentences under eight words", target: targetMetrics.shortSentencesUnder8Words, expected: distributions.shortSentencesUnder8Words });
  for (let index = 0; index < 4; index += 1) add(cadence, {
    name: `paragraph words, progression segment ${index + 1}`,
    target: targetMetrics.paragraphProgression?.[index]?.paragraphLengthWords?.median,
    expected: corpus.paragraphProgression?.[index]?.paragraphLengthWords,
    minimumSpread: 5
  });

  for (const [label, key] of [["contractions per 1,000 words", "contractionsPer1000Words"], ["questions per 1,000 words", "questionsPer1000Words"], ["list items per 1,000 words", "listItemsPer1000Words"]]) {
    add(voice, { name: label, target: targetMetrics[key], expected: distributions[key], minimumSpread: 1 });
  }
  for (const key of ["emDash", "semicolon", "colon", "parentheses"]) add(voice, {
    name: `${key} per 1,000 words`, target: targetMetrics.punctuationPer1000Words?.[key], expected: distributions.punctuationPer1000Words?.[key], minimumSpread: 1
  });

  add(lexicon, { name: "average word length", target: targetMetrics.averageWordLength, expected: distributions.averageWordLength, minimumSpread: 0.1 });
  add(lexicon, { name: "windowed unique-word ratio", target: targetMetrics.vocabulary?.windowedUniqueWordRatio, expected: distributions.windowedUniqueWordRatio, minimumSpread: 0.01 });

  for (const tag of corpus.wordTypes?.includedTags ?? []) add(pos, {
    name: `word type ${tag}, article percentage`,
    target: targetMetrics.wordTypes?.article?.percentages?.[tag],
    expected: corpus.wordTypes?.articlePercentages?.[tag],
    minimumSpread: 1
  });

  const trackedMeasurements = [];
  addIfPresent(trackedMeasurements, centeredDifference({
    name: "standalone labels per 1,000 words",
    target: targetMetrics.standaloneLabelsPer1000Words,
    expected: distributions.standaloneLabelsPer1000Words,
    minimumSpread: 1
  }));
  const findings = groups.flatMap((group) => group.findings.map((finding) => ({ ...finding, group: group.name, weight: group.weight })));
  const rankedFindings = [...findings].sort((a, b) => b.deviation - a.deviation || a.metric.localeCompare(b.metric));
  const groupScores = groups.map((group) => ({ group: group.name, weight: group.weight, metricCount: group.findings.length, deviation: rounded(mean(group.findings.map((finding) => finding.deviation))) }));
  return {
    target,
    proseWordCount: targetMetrics.proseWordCount,
    statisticalDeviationScore: rounded(weightedMean(groups)),
    groupScores,
    trackedMeasurements,
    strongestStatisticalDifferences: rankedFindings.slice(0, 5),
    metricComparisons: rankedFindings
  };
};

const comparisons = targets.map(compare).sort((a, b) => b.statisticalDeviationScore - a.statisticalDeviationScore || a.target.localeCompare(b.target));
process.stdout.write(`${JSON.stringify({
  schemaVersion: 2,
  scoring: {
    method: "robust centre distance grouped according to the profile's Verification calibration section",
    calibration,
    note: "Scores are heuristic review signals, not conformance grades or rewrite instructions."
  },
  profileMetrics: profilePath,
  comparisons
}, null, 2)}\n`);
