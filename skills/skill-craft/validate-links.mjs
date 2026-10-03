#!/usr/bin/env node
/**
 * Validate inline local Markdown links/images and heading anchors.
 * Usage: node validate-links.mjs <file.md> [...]
 * Requires Node.js 18+. No network or writes. Fenced examples, inline-code
 * examples, external URLs, and parameterized placeholder links are excluded.
 * This checks navigation, not Markdown rendering or workflow execution quality.
 */
import { readFile, stat } from "node:fs/promises";
import { dirname, resolve } from "node:path";

const files = process.argv.slice(2);
if (!files.length) {
  console.error("Usage: validate-links.mjs <file.md> [...]");
  process.exit(2);
}

function prose(text) {
  let fence = null;
  return text.split(/\r?\n/).map((line) => {
    const match = line.match(/^\s*(`{3,}|~{3,})/);
    if (match) {
      if (!fence) fence = match[1];
      else if (match[1][0] === fence[0] && match[1].length >= fence.length) fence = null;
      return "";
    }
    return fence ? "" : line;
  }).join("\n");
}

function anchors(text) {
  const counts = new Map();
  const result = new Set();
  for (const line of prose(text).split("\n")) {
    const heading = line.match(/^#{1,6}\s+(.+?)(?:\s+#+)?\s*$/)?.[1];
    if (!heading) continue;
    const base = heading.toLowerCase().replace(/[^\p{L}\p{N}_\-\s]/gu, "").replace(/\s/g, "-");
    const count = counts.get(base) || 0;
    counts.set(base, count + 1);
    result.add(count ? `${base}-${count}` : base);
  }
  return result;
}

let failed = false;
for (const file of files) {
  let checked = 0;
  try {
    const text = await readFile(file, "utf8");
    const content = prose(text).replace(/`[^`]*`/g, "");
    for (const match of content.matchAll(/!?\[[^\]]*\]\(([^\s)]+)\)/g)) {
      const target = match[1];
      if (/^[a-z][a-z0-9+.-]*:/i.test(target) || target.startsWith("//") || /[<>]/.test(target)) continue;
      checked += 1;
      try {
        const [path, fragment] = target.split("#");
        const destination = path ? resolve(dirname(file), decodeURIComponent(path)) : resolve(file);
        const info = await stat(destination);
        if (fragment && info.isFile()) {
          const headings = anchors(await readFile(destination, "utf8"));
          if (!headings.has(decodeURIComponent(fragment))) throw new Error(`missing heading anchor #${fragment}`);
        } else if (fragment) {
          throw new Error("heading anchor targets a directory");
        }
      } catch (error) {
        console.error(`${file}: ${target}: ${error.message}`);
        failed = true;
      }
    }
    console.log(`${file}: checked ${checked} local links`);
  } catch (error) {
    console.error(`${file}: ${error.message}`);
    failed = true;
  }
}
process.exitCode = failed ? 1 : 0;
