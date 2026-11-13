---
name: yt-dlp
description: "Download video or audio, inspect formats and metadata, retrieve subtitles, or run other yt-dlp actions on supported sites. Use when actual media is needed or YouTube caption retrieval fails; prefer youtube-transcript-download for caption-only requests. Includes FFmpeg processing of downloaded media, not speech recognition."
compatibility: "Python 3.10+ for bundled audio helper; yt-dlp executable or Python package; FFmpeg for media processing."
---

# yt-dlp

Use yt-dlp directly for requested action. Keep download and processing separate; use FFmpeg when processing is needed.

## Workflow

1. Choose command for requested action. If invocation supplies only a URL or no action, ask what result is needed; otherwise proceed without another confirmation. Examples use Python package; standalone `yt-dlp` on PATH also works. Use `python -m yt_dlp --help` for other actions and options.
   ```bash
   # Inspect formats without downloading
   python -m yt_dlp --ignore-config --no-playlist -F 'VIDEO_URL'

   # Download one video, merging streams with FFmpeg when needed
   python -m yt_dlp --ignore-config --no-playlist --no-overwrites -P './media' 'VIDEO_URL'

   # Retrieve subtitle tracks without downloading media
   python -m yt_dlp --ignore-config --no-playlist --no-overwrites --skip-download --write-subs --write-auto-subs --sub-langs en -P './media' 'VIDEO_URL'
   ```
   Use requested subtitle language; `en` above is only an example. Default to one video, no overwrites, and ignored user configuration. Enable playlists only when requested; ask before replacing existing files. Done when command matches requested scope.

2. Execute command. For original audio-only download, run bundled helper from this skill directory:
   ```bash
   python scripts/download-audio.py 'https://www.youtube.com/watch?v=cePFo4QpVoc' --output-dir './audio'
   ```
   Helper selects best available audio-only stream without conversion; extension depends on source format. Stdout returns JSON `{"path":"absolute/path/to/audio.webm"}`. Diagnostics go to stderr. Exit 0 means file exists and is non-empty; exit 1 reports a JSON error; usage errors exit 2 with argparse help on stderr. Existing files are not overwritten; yt-dlp may reuse an existing download. For direct commands, check exit status and requested metadata or output files. Correct a known option error and recheck; otherwise report blocker. Done when requested result is verified or blocker is explicit.

3. Use FFmpeg directly when processing is needed. Substitute actual input path and choose a new output filename:
   ```bash
   # MP3 for playback
   ffmpeg -nostdin -n -i 'audio.webm' -vn -c:a libmp3lame -q:a 2 'audio.mp3'

   # Mono 16 kHz PCM WAV for tools that require it
   ffmpeg -nostdin -n -i 'audio.webm' -vn -ac 1 -ar 16000 -c:a pcm_s16le 'audio.wav'

   # 30-second excerpt starting at 60 seconds
   ffmpeg -nostdin -n -ss 60 -i 'audio.webm' -t 30 -vn -c:a pcm_s16le 'excerpt.wav'
   ```
   Preserve source. Check FFmpeg exit status, then inspect output with `ffprobe -v error -show_entries format=duration:stream=codec_name,sample_rate,channels -of json 'audio.wav'`. Correct a known command error and recheck; otherwise report blocker. Done when requested output is verified.

4. Report result, file paths when applicable, and processing performed. If spoken text is needed, pass audio to an available speech-to-text tool; if none exists, state that limitation. Done when requested result is delivered or remaining capability gap is explicit.

## Rules

- Missing dependency: ask before installing or updating. `python -m pip install yt-dlp` installs Python package. FFmpeg is separate.
- No automatic browser cookies or credentials. Report download restrictions directly. Ask explicit approval before accessing browser cookies or authenticated sessions; do not bypass paywalls or access controls.
- Do not claim captions were transcribed from audio or that downloading audio means it was heard.
