#!/usr/bin/env bash
# Maintained native tooling checks. No network, only owned fixture repositories.
set -euo pipefail
cd "$(dirname "$0")/.."
source tools/native-build.sh
native_build tools/native-tool.c
export ATTENDANT_NATIVE_TOOL_HELPER=$NATIVE_HELPER
native_build tests/tools/check.c
exec "$NATIVE_HELPER"
