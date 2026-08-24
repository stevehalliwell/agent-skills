#!/usr/bin/env node
/**
 * Build the authoritative human-readable source map for one inquiry.
 * Usage: node build-source-map.mjs --index FILE --queue FILE --lineage FILE --output FILE
 */
import { mkdir, writeFile } from "node:fs/promises";
import path from "node:path";
import { hasHelp, readCsv, requiredOption } from "./lib/csv.mjs";
const args = process.argv.slice(2);
if (hasHelp(args)) { console.log("Usage: node build-source-map.mjs --index FILE --queue FILE --lineage FILE --output FILE"); process.exit(0); }
const indexPath = requiredOption(args, "--index"), queuePath = requiredOption(args, "--queue"), lineagePath = requiredOption(args, "--lineage"), outputPath = requiredOption(args, "--output");
const [index, queue, lineage] = await Promise.all([readCsv(indexPath), readCsv(queuePath), readCsv(lineagePath)]);
if (!index.length || index.some(row => !row.source_id || !row.canonical_url)) throw new Error("Index needs source_id and canonical_url.");
const queueBySource = Map.groupBy(queue, row => row.source_id);
const lineageBySource = new Map(lineage.map(row => [row.source_id, row]));
const safe = text => String(text || "").replaceAll("|", "\\|").replaceAll("\n", " ");
const linkArtifacts = text => String(text || "").split(" | ").filter(Boolean).map(item => `[artifact](${item})`).join(", ") || "—";
const rows = index.map(source => {
  const planned = queueBySource.get(source.source_id) || [], detail = lineageBySource.get(source.source_id) || {};
  return `| ${source.source_id} | [source](${source.canonical_url}) | ${safe(source.observation_ids)} | ${safe(source.titles)} | ${safe(source.source_types)} / ${safe(source.evidence_labels)} | ${safe(source.why_added || "Not supplied by discovery.")} | ${safe(planned.map(row => row.proposed_status || row.initial_status || "review-needed").join(" / "))} | ${safe(planned.map(row => row.selection_reason).filter(Boolean).join(" / ") || "—")} | ${safe(detail.capture_statuses || "not-captured")} | ${linkArtifacts(detail.artifacts)} | ${safe(detail.capture_notes || "—")} |`;
});
const text = `# Source Map\n\nThis is the authoritative human-readable mapping for this inquiry. Regenerate it after queue review or capture; raw discovery rows and capture records remain the underlying audit trail.\n\n| Source ID | Canonical source | Discovery origin | Title / label | Type / provenance | Why added | Fetch state | Fetch rationale | Capture status | Retained artifact | Notes / failure |\n| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |\n${rows.join("\n")}\n`;
await mkdir(path.dirname(outputPath), { recursive: true }); await writeFile(outputPath, text);
console.log(JSON.stringify({ output: outputPath, sources: index.length }, null, 2));
