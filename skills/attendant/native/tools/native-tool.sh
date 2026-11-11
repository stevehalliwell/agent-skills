#!/usr/bin/env bash
# Native dependency/provenance operations. Run --help for commands.
set -euo pipefail
cd "$(dirname "$0")/.."
source tools/native-build.sh
native_build tools/native-tool.c
exec "$NATIVE_HELPER" "$@"
