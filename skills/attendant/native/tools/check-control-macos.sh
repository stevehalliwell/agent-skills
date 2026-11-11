#!/usr/bin/env bash
# Real macOS control/Unix-socket acceptance; never opens existing workspaces.
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ $(uname -s) != Darwin ]]; then
  echo 'Requires native macOS. Runtime qualification pending; see TODO.md.' >&2
  echo '{"ok":false,"code":"unsupported-platform"}'; exit 2
fi
source tools/native-build.sh
native_build tests/gui/control-macos-check.c
mkdir -p build/control-check
export ATTENDANT_CONTROL_DIRECTORY
ATTENDANT_CONTROL_DIRECTORY=$(mktemp -d build/control-check/run-XXXXXX)
exec "$NATIVE_HELPER" "$@"
