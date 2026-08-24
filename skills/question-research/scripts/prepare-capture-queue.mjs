#!/usr/bin/env node
/**
 * Create a non-filtering capture queue from a normalised source index.
 * Usage: node prepare-capture-queue.mjs --index FILE --output FILE
 */
import { mkdir } from "node:fs/promises";
import path from "node:path";
import { hasHelp, readCsv, requiredOption, writeCsv } from "./lib/csv.mjs";

const args = process.argv.slice(2);
if (hasHelp(args)) {
  console.log("Usage: node prepare-capture-queue.mjs --index FILE --output FILE");
  process.exit(0);
}
const indexPath = requiredOption(args, "--index");
const outputPath = requiredOption(args, "--output");
const sources = await readCsv(indexPath);
if (!sources.length || sources.some(source => !source.source_id || !source.canonical_url)) throw new Error("Index needs source_id and canonical_url columns with at least one row.");
const headers = ["queue_id", "source_id", "canonical_url", "initial_status", "recommended_method", "review_priority", "observation_ids", "source_types", "evidence_labels", "titles", "why_added", "observation_count", "selection_reason", "thread_page_range", "review_notes"];
const queue = sources.map((source, index) => ({ queue_id: `CQ-${String(index + 1).padStart(4, "0")}`, source_id: source.source_id, canonical_url: source.canonical_url, initial_status: "review-needed", recommended_method: "", review_priority: source.review_priority, observation_ids: source.observation_ids, source_types: source.source_types, evidence_labels: source.evidence_labels, titles: source.titles, why_added: source.why_added || "", observation_count: source.observation_count, selection_reason: "", thread_page_range: "", review_notes: "" }));
await mkdir(path.dirname(outputPath), { recursive: true });
await writeCsv(outputPath, queue, headers);
console.log(JSON.stringify({ index: indexPath, output: outputPath, queue_rows: queue.length, status: "review-needed" }, null, 2));
