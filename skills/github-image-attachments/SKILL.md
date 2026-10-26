---
name: github-image-attachments
description: "Attach or embed local images and screenshots in GitHub PR descriptions, issues, or comments. Use when posting visual evidence or when installed gh appears unable to upload images. Upload through GitHub CLI and verify inline attachments; skip repository image commits and release assets."
---

# GitHub Image Attachments

Use `gh --attach`: uploads local images and replaces matching Markdown paths with GitHub-hosted attachment URLs. No image commit or browser required. Requires push access to target repository.

## Workflow

1. Inspect intended images locally. Remove secrets; crop/resize copies only as needed. Put images and UTF-8 `body.md` in one staging directory. Use descriptive alt text:

   ```markdown
   ## Screenshot
   ![Searchable prototype grid filtered to sand levels](sand-menu.png)
   ```

2. Run `gh pr comment --help` or help for intended command. Check for `--attach`, not just installed version. If absent, use a current official CLI release. Original successful run used portable **2.103.0**: downloaded platform archive and published checksums from `cli/cli` releases, verified SHA-256, extracted into temporary directory, then invoked that executable directly. Leave global installation unchanged. Verify replacement executable exposes `--attach`. Never print auth tokens.

3. Confirm target repository, issue/PR, and posting authority. Preparing files does not authorize uploading or posting. Once authorized, run from staging directory, substituting actual target:

   ```sh
   gh pr comment NUMBER --repo OWNER/REPO --body-file body.md \
     --attach ./sand-menu.png
   ```

   Repeat `--attach` for each image. Matching local Markdown references are rewritten in place; unreferenced attachments append at end. Same flag works with `gh issue comment`, `gh issue create/edit`, and `gh pr create/edit`. Use selected executable's help for command-specific arguments. A local path alone does not upload anything.

4. Read back posted body through `gh api` or `gh pr/issue view`. Verify local references became attachment URLs and each intended image renders inline. Return comment/issue/PR URL. If result is uncertain, inspect existing post before retrying; do not duplicate comments.

## Evidence

- [GitHub attachment documentation](https://docs.github.com/en/github-cli/github-cli/attaching-files-with-github-cli).
- [Successful original comment](https://github.com/ActuatorDigital/godot-proto/pull/1#issuecomment-6330512300): staged Markdown plus two PNGs; portable CLI 2.103.0 posted both with `--body-file` and repeated `--attach`. Read-back confirmed `https://github.com/user-attachments/assets/…` URLs.

Missing `--attach` in an old CLI is a version limitation, not proof that GitHub cannot accept images.
