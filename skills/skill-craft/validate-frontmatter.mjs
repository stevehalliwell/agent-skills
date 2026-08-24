#!/usr/bin/env node

import { readFile } from "node:fs/promises";

const files = process.argv.slice(2);

if (!files.length) {
  console.error("Usage: validate-frontmatter.mjs <SKILL.md> [...]");
  process.exit(2);
}

let failed = false;

for (const file of files) {
  let text;
  try {
    text = await readFile(file, "utf8");
  } catch (error) {
    console.error(`${file}: ${error.message}`);
    failed = true;
    continue;
  }

  const frontmatter = text.match(/^---\r?\n([\s\S]*?)\r?\n---(?:\r?\n|$)/);
  if (!frontmatter) {
    console.error(`${file}: missing YAML frontmatter`);
    failed = true;
    continue;
  }

  const name = frontmatter[1].match(/^name:\s*(.+)\s*$/m)?.[1];
  const descriptionLine = frontmatter[1].match(/^description:\s*(.+)\s*$/m)?.[1];
  const description = descriptionLine?.match(/^(["'])([\s\S]*)\1$/)?.[2];
  const errors = [];

  if (!name) errors.push("missing name");
  else if (name.length > 64) errors.push("name exceeds 64 characters");
  else if (!/^[a-z0-9-]+$/.test(name)) errors.push("name must use lowercase letters, numbers, and hyphens");

  if (!descriptionLine) errors.push("missing description");
  else if (description === undefined) errors.push("description must be quoted");
  else if (description.length > 1024) errors.push("description exceeds 1024 characters");

  if (errors.length) {
    console.error(`${file}: ${errors.join("; ")}`);
    failed = true;
  } else {
    console.log(`${file}: OK`);
  }
}

process.exitCode = failed ? 1 : 0;
