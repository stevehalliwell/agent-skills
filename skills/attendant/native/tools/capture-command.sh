#!/usr/bin/env bash
# Internal ordinary-command capture. Caller exclusively owns stdout file.
# Stderr stays with parent log; argv never passes through shell evaluation.
set -euo pipefail
output=$1
errors=$2
shift 2
"$@" > "$output" 2> "$errors"
