#!/usr/bin/env node

import { readFile } from "node:fs/promises";

const [file] = process.argv.slice(2);

if (!file || process.argv.length > 3) {
  console.error("Usage: md-words.mjs <file.md>");
  process.exit(2);
}

let text;
try {
  text = await readFile(file, "utf8");
} catch (error) {
  console.error(`${file}: ${error.message}`);
  process.exit(1);
}

text = text.replace(/^---\r?\n[\s\S]*?\r?\n---(?:\r?\n|$)/, "");
const words = text.trim() ? text.trim().split(/\s+/).length : 0;
console.log(words);
