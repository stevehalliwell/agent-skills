#!/usr/bin/env node
/**
 * Replace the generated source/capture-status view in a research register.
 * Usage: node update-research-register.mjs --register FILE --lineage FILE
 *
 * Manual sections remain untouched. This script owns only its marked block.
 */
import { readFile, writeFile } from "node:fs/promises";
import path from "node:path";
import { hasHelp, readCsv, requiredOption } from "./lib/csv.mjs";

const args = process.argv.slice(2);
if (hasHelp(args)) {
  console.log("Usage: node update-research-register.mjs --register FILE --lineage FILE");
  process.exit(0);
}
const registerPath = requiredOption(args, "--register");
const lineagePath = requiredOption(args, "--lineage");
const [register, lineage] = await Promise.all([readFile(registerPath, "utf8"), readCsv(lineagePath)]);
if (!lineage.length || lineage.some(row => !row.source_id || !row.canonical_url || !("capture_statuses" in row))) throw new Error("Lineage needs source_id, canonical_url, and capture_statuses columns.");
const escapeCell = value => String(value || "").replaceAll("|", "\\|");
const link = (label, target) => {
  if (!target) return "";
  if (/^https?:/.test(target)) return `[${label}](${target})`;
  const relative = path.relative(path.dirname(registerPath), path.resolve(path.dirname(lineagePath), target)).split(path.sep).join("/");
  return `[${label}](${relative})`;
};
const block = [
  "<!-- question-research:source-capture-status:start -->",
  "## Generated source/capture status",
  "",
  "Generated from the canonical lineage CSV. Re-run this script after capture records change; manual source context remains in **Source lineage**.",
  "",
  "| Source | URL | Queue | Capture status | Method | Artifact | Captured at |",
  "| --- | --- | --- | --- | --- | --- | --- |",
  ...lineage.map(row => `| ${escapeCell(row.source_id)} | ${link("source", row.canonical_url)} | ${escapeCell(row.queue_ids)} | ${escapeCell(row.capture_statuses)} | ${escapeCell(row.methods)} | ${row.artifacts ? row.artifacts.split(" | ").map((artifact, index) => link(`artifact ${index + 1}`, artifact)).join("<br>") : ""} | ${escapeCell(row.captured_at)} |`),
  "",
  "<!-- question-research:source-capture-status:end -->",
  "",
].join("\n");
const pattern = /<!-- question-research:source-capture-status:start -->[\s\S]*?<!-- question-research:source-capture-status:end -->\n?/;
const updated = pattern.test(register) ? register.replace(pattern, block) : `${register.trimEnd()}\n\n${block}`;
await writeFile(registerPath, updated);
console.log(JSON.stringify({ register: registerPath, lineage: lineagePath, sources: lineage.length, generated_block: "updated" }, null, 2));
