#!/usr/bin/env bash
# Native warm synthetic CLI process benchmark. --help describes timing scope.
set -euo pipefail
cd "$(dirname "$0")/.."
source tools/native-build.sh
native_build tools/benchmark.c
mkdir -p build/bench
export ATTENDANT_BENCHMARK_DIRECTORY
ATTENDANT_BENCHMARK_DIRECTORY=$(mktemp -d build/bench/run-XXXXXX)
exec "$NATIVE_HELPER" "$@"
