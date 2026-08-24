#!/usr/bin/env node
/**
 * Fetch a YouTube caption transcript.
 *
 * Usage:
 *   node scripts/download.mjs <youtube-url-or-id> [--lang <code>] [--format text|json] [--output <path>]
 */
import { writeFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fetchTranscript } from "youtube-transcript";

function usage(error) {
  if (error) console.error(`Error: ${error}\n`);
  console.error("Usage: download.mjs <youtube-url-or-id> [--lang <code>] [--format text|json] [--output <path>");
  process.exit(error ? 1 : 0);
}

function extractVideoId(input) {
  if (/^[a-zA-Z0-9_-]{11}$/.test(input)) return input;

  const patterns = [
    /[?&]v=([a-zA-Z0-9_-]{11})/i,
    /youtu\.be\/([a-zA-Z0-9_-]{11})/i,
    /(?:www\.)?youtube\.com\/(?:embed|v|e)\/([a-zA-Z0-9_-]{11})/i,
    /(?:www\.)?youtube\.com\/(?:shorts|live)\/([a-zA-Z0-9_-]{11})(?:\/|$)/i,
  ];

  for (const pattern of patterns) {
    const match = input.match(pattern);
    if (match) return match[1];
  }
  return input;
}

function formatOffset(offsetMs) {
  const totalSeconds = Math.floor(offsetMs / 1000);
  const hours = Math.floor(totalSeconds / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  const seconds = totalSeconds % 60;
  return hours > 0
    ? `[${hours}:${String(minutes).padStart(2, "0")}:${String(seconds).padStart(2, "0")}]`
    : `[${minutes}:${String(seconds).padStart(2, "0")}]`;
}

const args = process.argv.slice(2);
if (args.includes("--help") || args.includes("-h")) usage();
if (!args[0] || args[0].startsWith("-")) usage("A YouTube URL or 11-character video ID is required.");

const input = args.shift();
let lang;
let format = "text";
let output;

while (args.length > 0) {
  const option = args.shift();
  const value = args.shift();
  if (!value || value.startsWith("--")) usage(`Missing value for ${option}.`);
  if (option === "--lang") lang = value;
  else if (option === "--format" && ["text", "json"].includes(value)) format = value;
  else if (option === "--format") usage("Format must be text or json.");
  else if (option === "--output") output = value;
  else usage(`Unknown option ${option}.`);
}

const videoId = extractVideoId(input);
const segments = await fetchTranscript(videoId, lang ? { lang } : undefined);
if (segments.length === 0) throw new Error("No transcript available for this video.");

const result = format === "json"
  ? JSON.stringify({
      videoId,
      language: segments[0]?.lang,
      segments: segments.map(({ text, duration, offset }) => ({ text, duration, offset })),
    }, null, 2)
  : segments.map((segment) => `${formatOffset(segment.offset)} ${segment.text}`).join("\n");

if (output) {
  const path = resolve(output);
  await writeFile(path, `${result}\n`, "utf8");
  console.error(`Saved transcript to ${path}`);
} else {
  process.stdout.write(`${result}\n`);
}
