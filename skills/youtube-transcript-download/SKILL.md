---
name: youtube-transcript-download
description: "Download or fetch a YouTube video transcript, captions, subtitles, or timestamped spoken text from a YouTube URL or video ID. Use when a transcript needs to be retrieved or saved locally; then summarize or reformat it only when requested."
compatibility: "Node.js 18+; install the bundled npm dependency once."
---

# YouTube Transcript Download

Fetch a YouTube caption transcript with a local script rather than the Pi extension.

## Workflow

1. From this skill directory, check whether `node_modules/youtube-transcript` exists. If absent, state that `npm install` is needed and ask approval before installing this dependency. After approval:
   ```bash
   npm install
   ```
   Done when the dependency is available or the installation decision/blocker is explicit.

2. Fetch the transcript. The default output matches the former extension: one caption per line as `[MM:SS] text`.
   ```bash
   node scripts/download.mjs '<youtube-url-or-video-id>'
   node scripts/download.mjs '<youtube-url-or-video-id>' --lang en
   ```
   Done when timestamped transcript text is returned.

3. Save it only when the user requests a local file; use JSON when raw caption metadata is needed.
   ```bash
   node scripts/download.mjs '<youtube-url-or-video-id>' --output transcript.txt
   node scripts/download.mjs '<youtube-url-or-video-id>' --format json --output transcript.json
   ```
   Done when the requested file exists and its path is reported.

4. Use the retrieved transcript as source material for requested summaries, notes, outlines, or other transformations. Preserve timestamps when the user needs to trace claims back to the video.
   Done when the requested output is delivered.

## Rules

- Accept standard YouTube watch, short, live, embed, and shortened URLs, plus bare 11-character IDs.
- Request a language with `--lang`; without it, use the first available transcript.
- Report unavailable captions or retrieval errors directly; do not imply the video was transcribed from audio.
