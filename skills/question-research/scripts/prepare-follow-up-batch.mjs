#!/usr/bin/env node
/**
 * Select explicitly reviewed sources into a bounded follow-up batch.
 * Usage: node prepare-follow-up-batch.mjs --queue FILE --output FILE [--limit N]
 *
 * The input queue must have proposed_status, source_id, and canonical_url
 * fields. It is not modified.
 */
import { mkdir } from "node:fs/promises";
import path from "node:path";
import { hasHelp, readCsv, requiredOption, writeCsv } from "./lib/csv.mjs";

const args = process.argv.slice(2);
if (hasHelp(args)) {
  console.log("Usage: node prepare-follow-up-batch.mjs --queue FILE --output FILE [--limit N]");
  process.exit(0);
}
const queuePath = requiredOption(args, "--queue");
const outputPath = requiredOption(args, "--output");
const limitValue = args.includes("--limit") ? requiredOption(args, "--limit") : "15";
const limit = Number.parseInt(limitValue, 10);
if (!Number.isInteger(limit) || limit < 1) throw new Error("--limit must be a positive integer.");
const targetStatus = "crawl4ai-candidate";
const queue = await readCsv(queuePath);
if (queue.some(row => !row.source_id || !row.canonical_url || !("proposed_status" in row))) throw new Error("Queue needs proposed_status, source_id, and canonical_url columns.");
const selected = queue.filter(row => row.proposed_status === targetStatus).sort((a, b) => Number(b.review_priority) - Number(a.review_priority) || a.queue_id.localeCompare(b.queue_id)).slice(0, limit);
if (!selected.length) throw new Error(`No ${targetStatus} rows in ${queuePath}.`);
const headers = ["queue_id", "source_id", "url", "title", "capture_reason", "thread_page_range"];
const rows = selected.map(row => ({ queue_id: row.queue_id, source_id: row.source_id, url: row.canonical_url, title: row.titles, capture_reason: row.selection_reason, thread_page_range: row.thread_page_range }));
await mkdir(path.dirname(outputPath), { recursive: true });
await writeCsv(outputPath, rows, headers);
console.log(JSON.stringify({ queue: queuePath, output: outputPath, method: "crawl4ai", selected: rows.length, limit, source_status: targetStatus }, null, 2));
