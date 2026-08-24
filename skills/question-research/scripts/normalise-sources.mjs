#!/usr/bin/env node
/**
 * Non-destructively normalise question-research discovery CSVs.
 * Usage: node normalise-sources.mjs --input-dir DIR --output-dir DIR
 *
 * Reads every immediate child DIR/source-candidates.csv and writes every input
 * observation plus an exact-canonical-URL index and merge audit. No rows are
 * discarded or reclassified.
 */
import { mkdir, readdir, readFile, writeFile } from "node:fs/promises";
import path from "node:path";
import { hasHelp, toCsv, parseCsv, requiredOption } from "./lib/csv.mjs";

const args = process.argv.slice(2);
if (hasHelp(args)) {
  console.log("Usage: node normalise-sources.mjs --input-dir DIR --output-dir DIR");
  process.exit(0);
}
const inputDir = requiredOption(args, "--input-dir");
const outputDir = requiredOption(args, "--output-dir");
const tracking = new Set(["fbclid", "gclid", "dclid", "msclkid", "mc_cid", "mc_eid"]);
const canonicalUrl = value => {
  try {
    const url = new URL(value.trim());
    url.hash = ""; url.hostname = url.hostname.toLowerCase();
    for (const key of [...url.searchParams.keys()]) if (key.toLowerCase().startsWith("utm_") || tracking.has(key.toLowerCase())) url.searchParams.delete(key);
    url.searchParams.sort();
    if (url.pathname === "/") url.pathname = "";
    return url.toString().replace(/\/$/, "");
  } catch { return value.trim(); }
};
const priority = record => {
  const source = (record.source_type || "").toLowerCase();
  const label = (record.evidence_label || record.evidence_strength || "").toLowerCase();
  const sourcePoints = /official|documentation|issue tracker/.test(source) ? 3 : /developer|postmortem|expert|forum/.test(source) ? 2 : 1;
  return sourcePoints + ({ primary: 3, strong: 3, "expert-secondary": 2, moderate: 2, "reported-secondary": 1, "community-report": 1, "discovery-signal": 1, discovery: 1 }[label] ?? 0);
};

const directories = (await readdir(inputDir, { withFileTypes: true })).filter(entry => entry.isDirectory()).sort((a, b) => a.name.localeCompare(b.name));
const observations = [];
for (const directory of directories) {
  const candidate = path.join(inputDir, directory.name, "source-candidates.csv");
  try {
    const rows = parseCsv(await readFile(candidate, "utf8"));
    for (const [index, record] of rows.entries()) {
      if (!["url", "title", "source_type", "evidence_label", "why_retain"].every(field => record[field]?.trim())) throw new Error(`${candidate} row ${index + 2} needs url, title, source_type, evidence_label, and why_retain.`);
      observations.push({ observation_id: `${directory.name}:${index + 2}`, batch: directory.name, input_row: index + 2, canonical_url: canonicalUrl(record.url), review_priority: priority(record), ...record, raw_record_json: JSON.stringify(record) });
    }
  } catch (error) { if (error.code !== "ENOENT") throw error; }
}
if (!observations.length) throw new Error(`No source-candidates.csv files found below ${inputDir}`);
const groups = Map.groupBy(observations, item => item.canonical_url);
const sources = [...groups.entries()].map(([url, rows], index) => {
  const values = key => [...new Set(rows.map(row => row[key]).filter(Boolean))].join(" | ");
  return { source_id: `SRC-${String(index + 1).padStart(4, "0")}`, canonical_url: url, observation_count: rows.length, review_priority: Math.max(...rows.map(row => row.review_priority)), batches: values("batch"), observation_ids: values("observation_id"), source_types: values("source_type"), evidence_labels: values("evidence_label") || values("evidence_strength"), titles: values("title"), why_added: values("why_retain") || values("why_added"), original_urls: values("url"), merge_note: rows.length > 1 ? "Exact canonical URL match; all observations retained." : "Single observation." };
}).sort((a, b) => b.review_priority - a.review_priority || a.canonical_url.localeCompare(b.canonical_url));
const sourceByUrl = new Map(sources.map(source => [source.canonical_url, source.source_id]));
for (const observation of observations) observation.source_id = sourceByUrl.get(observation.canonical_url);
const report = { generated_at: new Date().toISOString(), input_dir: inputDir, input_observations: observations.length, unique_canonical_urls: sources.length, exact_url_merge_groups: sources.filter(source => source.observation_count > 1).length, ranking_note: "review_priority is a non-filtering sort aid, not a source-quality verdict." };
await mkdir(outputDir, { recursive: true });
await writeFile(path.join(outputDir, "source-observations.csv"), toCsv(observations, [...new Set(observations.flatMap(Object.keys))]));
await writeFile(path.join(outputDir, "source-index.csv"), toCsv(sources, Object.keys(sources[0])));
await writeFile(path.join(outputDir, "dedup-report.json"), `${JSON.stringify(report, null, 2)}\n`);
console.log(JSON.stringify({ output_dir: outputDir, ...report }, null, 2));
