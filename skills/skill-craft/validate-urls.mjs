#!/usr/bin/env node

/**
 * Validate HTTP(S) URLs found in Markdown files.
 *
 * Usage: node ./validate-urls.mjs <file.md> [...]
 * Exits non-zero for malformed URLs, request failures, or non-2xx/3xx responses.
 */

import { readFile } from "node:fs/promises";

const files = process.argv.slice(2);
const urlPattern = /https?:\/\/[^\s<>"'`\])}]*/gi;
const timeoutMs = 15_000;

if (!files.length) {
  console.error("Usage: validate-urls.mjs <file.md> [...]");
  process.exit(2);
}

function urlsIn(text) {
  return [...new Set(text.match(urlPattern) ?? [])];
}

async function checkUrl(value) {
  let url;
  try {
    url = new URL(value);
  } catch {
    return `malformed URL: ${value}`;
  }

  if (url.protocol !== "http:" && url.protocol !== "https:") {
    return `unsupported URL protocol: ${value}`;
  }

  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), timeoutMs);

  try {
    let response = await fetch(url, {
      method: "HEAD",
      redirect: "follow",
      signal: controller.signal,
    });

    if (response.status === 405 || response.status === 501) {
      response = await fetch(url, {
        method: "GET",
        headers: { Range: "bytes=0-0" },
        redirect: "follow",
        signal: controller.signal,
      });
    }

    if (response.status >= 200 && response.status < 400) return null;
    return `HTTP ${response.status}: ${value}`;
  } catch (error) {
    const reason = error.name === "AbortError" ? `timed out after ${timeoutMs / 1000}s` : error.message;
    return `${reason}: ${value}`;
  } finally {
    clearTimeout(timer);
  }
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

  const urls = urlsIn(text);
  const results = await Promise.all(urls.map(checkUrl));
  const errors = results.filter(Boolean);

  if (errors.length) {
    for (const error of errors) console.error(`${file}: ${error}`);
    failed = true;
  } else {
    console.log(`${file}: OK (${urls.length} HTTP(S) URL${urls.length === 1 ? "" : "s"})`);
  }
}

process.exitCode = failed ? 1 : 0;
