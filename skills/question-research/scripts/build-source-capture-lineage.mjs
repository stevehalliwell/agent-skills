#!/usr/bin/env node
/**
 * Build a complete source-to-capture lineage view without removing uncaptured sources.
 * Usage: node build-source-capture-lineage.mjs --index FILE --queue FILE --capture-records FILE --output FILE
 *
 * capture-records.csv needs queue_id,status,method,artifact_path,captured_at.
 */
import { mkdir } from "node:fs/promises";
import path from "node:path";
import { hasHelp, readCsv, requiredOption, writeCsv } from "./lib/csv.mjs";

const args = process.argv.slice(2);
if (hasHelp(args)) {
  console.log("Usage: node build-source-capture-lineage.mjs --index FILE --queue FILE --capture-records FILE --output FILE");
  process.exit(0);
}
const indexPath = requiredOption(args, "--index");
const queuePath = requiredOption(args, "--queue");
const capturePath = requiredOption(args, "--capture-records");
const outputPath = requiredOption(args, "--output");
const [sources, queue, captures] = await Promise.all([readCsv(indexPath), readCsv(queuePath), readCsv(capturePath)]);
if (!sources.length || sources.some(row => !row.source_id || !row.canonical_url)) throw new Error("Index needs source_id and canonical_url columns.");
if (queue.some(row => !row.queue_id || !row.source_id)) throw new Error("Queue needs queue_id and source_id columns.");
if (captures.some(row => !row.queue_id || !row.status || !row.method || !("artifact_path" in row) || !("captured_at" in row))) throw new Error("Capture records need queue_id,status,method,artifact_path,captured_at columns.");
const queueBySource = Map.groupBy(queue, row => row.source_id);
const capturesByQueue = Map.groupBy(captures, row => row.queue_id);
const joined = value => [...new Set(value.filter(Boolean))].join(" | ");
const rows = sources.map(source => {
  const sourceQueue = queueBySource.get(source.source_id) || [];
  const records = sourceQueue.flatMap(item => capturesByQueue.get(item.queue_id) || []);
  return {
    source_id: source.source_id,
    canonical_url: source.canonical_url,
    observation_ids: source.observation_ids || "",
    queue_ids: joined(sourceQueue.map(item => item.queue_id)),
    queue_statuses: joined(sourceQueue.map(item => item.proposed_status || item.initial_status || "review-needed")),
    capture_count: records.length,
    capture_statuses: joined(records.map(item => item.status)) || "not-captured",
    methods: joined(records.map(item => item.method)),
    artifacts: joined(records.map(item => item.artifact_path)),
    captured_at: joined(records.map(item => item.captured_at)),
    capture_notes: joined(records.map(item => item.detail || item.notes || "")),
  };
});
await mkdir(path.dirname(outputPath), { recursive: true });
await writeCsv(outputPath, rows, Object.keys(rows[0]));
console.log(JSON.stringify({ index: indexPath, queue: queuePath, capture_records: capturePath, output: outputPath, sources: rows.length, captured_sources: rows.filter(row => row.capture_count > 0).length, uncaptured_sources: rows.filter(row => row.capture_count === 0).length }, null, 2));
