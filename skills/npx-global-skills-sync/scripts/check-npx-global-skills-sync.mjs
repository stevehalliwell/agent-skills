#!/usr/bin/env node
/**
 * Compare npx global skill state with Pi's tracked global-skill manifest.
 *
 * Usage: node check-npx-global-skills-sync.mjs [--json]
 * Exit: 0 aligned, 1 differences found, 2 invalid arguments or unreadable state.
 */

import fs from "node:fs";
import os from "node:os";
import path from "node:path";

const args = process.argv.slice(2);
const jsonOutput = args.includes("--json");
if (args.some((arg) => arg !== "--json" && arg !== "--help")) {
  console.error("Usage: node check-npx-global-skills-sync.mjs [--json]");
  process.exit(2);
}
if (args.includes("--help")) {
  console.log("Usage: node check-npx-global-skills-sync.mjs [--json]");
  process.exit(0);
}

const home = os.homedir();
const paths = {
  npxLock: path.join(home, ".agents", ".skill-lock.json"),
  npxSkills: path.join(home, ".agents", "skills"),
  manifest: path.join(home, ".pi", "agent", "skills_sync.json"),
  piSkills: path.join(home, ".pi", "agent", "skills"),
};
const trackedFields = ["source", "sourceType", "sourceUrl", "skillPath", "skillFolderHash"];

function readJson(filePath, label) {
  try {
    return JSON.parse(fs.readFileSync(filePath, "utf8"));
  } catch (error) {
    throw new Error(`Cannot read ${label} at ${filePath}: ${error.message}`);
  }
}

function isInside(parent, candidate) {
  const relative = path.relative(parent, candidate);
  return relative === "" || (!relative.startsWith(`..${path.sep}`) && relative !== "..");
}

function resolveDirectory(directory, label) {
  try {
    return fs.realpathSync(directory);
  } catch (error) {
    throw new Error(`Cannot resolve ${label} at ${directory}: ${error.message}`);
  }
}

function collectManagedLinks(piSkills, npxSkills) {
  const links = {};
  for (const entry of fs.readdirSync(piSkills, { withFileTypes: true })) {
    const entryPath = path.join(piSkills, entry.name);
    if (!fs.lstatSync(entryPath).isSymbolicLink()) continue;

    const target = fs.realpathSync(entryPath);
    if (isInside(npxSkills, target)) links[entry.name] = target;
  }
  return links;
}

try {
  const lock = readJson(paths.npxLock, "npx lock");
  const manifest = readJson(paths.manifest, "Pi manifest");
  if (!lock.skills || typeof lock.skills !== "object" || !manifest.skills || typeof manifest.skills !== "object") {
    throw new Error("Both JSON files must contain a skills object.");
  }

  const npxSkills = resolveDirectory(paths.npxSkills, "npx skills directory");
  const piSkills = resolveDirectory(paths.piSkills, "Pi skills directory");
  const managedLinks = collectManagedLinks(piSkills, npxSkills);
  const names = new Set([...Object.keys(lock.skills), ...Object.keys(manifest.skills), ...Object.keys(managedLinks)]);
  const differences = [];

  for (const name of [...names].sort()) {
    const npxSkill = lock.skills[name];
    const manifestSkill = manifest.skills[name];
    const classifications = [];
    const changedFields = [];

    if (npxSkill && !manifestSkill) classifications.push("npx-only");
    if (manifestSkill && !npxSkill) classifications.push("manifest-only");
    if (npxSkill && manifestSkill) {
      for (const field of trackedFields) {
        if ((npxSkill[field] ?? null) !== (manifestSkill[field] ?? null)) changedFields.push(field);
      }
      if (changedFields.length) classifications.push("metadata-mismatch");
    }
    if (npxSkill && !managedLinks[name]) classifications.push("link-mismatch");
    if (managedLinks[name] && !npxSkill) classifications.push("orphaned-pi-link");

    if (classifications.length) differences.push({ name, classifications, changedFields });
  }

  const result = { paths, managedLinks, differences, aligned: differences.length === 0 };
  if (jsonOutput) {
    console.log(JSON.stringify(result, null, 2));
  } else {
    console.log("Managed Pi links:");
    for (const [name, target] of Object.entries(managedLinks)) console.log(`- ${name}: ${target}`);
    console.log(differences.length ? `Differences: ${JSON.stringify(differences)}` : "Differences: none");
  }
  process.exit(differences.length ? 1 : 0);
} catch (error) {
  console.error(error.message);
  process.exit(2);
}
