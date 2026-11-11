#!/usr/bin/env bash
# Shared C GUI acceptance. Existing builds required; never acquires dependencies.
set -euo pipefail
cd "$(dirname "$0")/.."
source tools/native-build.sh
native_build tests/gui/coordinator.c
mkdir -p build/gui-check
export ATTENDANT_GUI_REPORT_DIRECTORY
ATTENDANT_GUI_REPORT_DIRECTORY=$(mktemp -d build/gui-check/run-XXXXXX)
exec "$NATIVE_HELPER" "$@"
