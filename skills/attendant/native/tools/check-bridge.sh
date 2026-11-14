#!/usr/bin/env bash
# Complete macOS bridge acceptance/report; lower-level builder remains separate.
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ $(uname -s) != Darwin ]]; then
  echo 'Requires native macOS; cannot run bridge acceptance on this host.' >&2
  echo '{"ok":false,"code":"unsupported-platform"}'; exit 2
fi
source tools/native-build.sh
native_build tests/gui/bridge-coordinator.c
mkdir -p build/bridge-check
export ATTENDANT_BRIDGE_DIRECTORY
ATTENDANT_BRIDGE_DIRECTORY=$(mktemp -d build/bridge-check/run-XXXXXX)
exec "$NATIVE_HELPER" "$@"
