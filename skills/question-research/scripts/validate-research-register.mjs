#!/usr/bin/env node
/**
 * Validate a project-local question-research register and its local Markdown links.
 * Usage: node validate-research-register.mjs --register FILE
 */
import { access, readFile } from "node:fs/promises";
import path from "node:path";
import { hasHelp, requiredOption } from "./lib/csv.mjs";

const args = process.argv.slice(2);
if (hasHelp(args)) {
  console.log("Usage: node validate-research-register.mjs --register FILE");
  process.exit(0);
}
const register = requiredOption(args, "--register");
const text = await readFile(register, "utf8");
const errors = [];
const frontmatter = text.match(/^---\n([\s\S]*?)\n---/);
if (!frontmatter) errors.push("Missing YAML frontmatter.");
else for (const field of ["question_id", "question", "outcome", "status", "created_at", "updated_at"]) if (!new RegExp(`^${field}:\\s*\\S`, "m").test(frontmatter[1])) errors.push(`Missing frontmatter field: ${field}.`);
const outcome = frontmatter?.[1].match(/^outcome:\s*(\S+)/m)?.[1];
if (outcome && !new Set(["evidence-answer", "evidence-dossier"]).has(outcome)) errors.push(`Unsupported outcome: ${outcome}.`);
for (const heading of ["## Intent and boundary", "## First blush", "## Discovery batches", "## Source lineage", "## Evidence matrix", "## Conflicts and gaps", "## Synthesis", "## Validation"]) if (!text.includes(heading)) errors.push(`Missing required section: ${heading}.`);
if (!/\[source map\]\(source-map\.md\)/i.test(text)) errors.push("Missing authoritative source-map.md link.");
for (const match of text.matchAll(/\[[^\]]*\]\(([^)]+)\)/g)) {
  const target = match[1].split("#")[0];
  if (!target || /^(https?:|mailto:)/.test(target)) continue;
  try { await access(path.resolve(path.dirname(register), target)); }
  catch { errors.push(`Missing local link target: ${target}`); }
}
if (errors.length) {
  console.error(JSON.stringify({ register, valid: false, errors }, null, 2));
  process.exit(1);
}
console.log(JSON.stringify({ register, valid: true, outcome, local_links: "resolved" }, null, 2));
