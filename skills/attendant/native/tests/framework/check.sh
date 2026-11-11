# Internal helper sourced by test.sh. Uses its compile function/run directory.
# Each scenario has an owned directory; expected failures remain as evidence.
framework_check_tsv() {
  local file=$1 planned=$2 rows=$3 failures=$4 completed=$5
  awk -F '\t' -v planned="$planned" -v rows="$rows" -v failures="$failures" -v completed="$completed" '
    $1 == "kind" { headers++; next }
    $1 == "suite" { suites++; if ($2 != planned) bad=1; next }
    $1 == "test" { tests++; if ($2 == "fail") failed++; else if ($2 != "ok") bad=1; next }
    $1 == "complete" { endings++; if ($2 != failures || $3 != rows || $4 != rows-failures) bad=1; next }
    { bad=1 }
    END { exit (bad || headers != 1 || suites != 1 || tests != rows || failed != failures || endings != completed) }
  ' "$file"
}
check_test_framework() {
  local directory="$run/framework" generator="$run/framework/generator$executable_suffix"
  local runner="$run/framework/runner$executable_suffix" scenario name status expected rows failures completed first_failed suite_failed
  mkdir "$directory"
  if ! compile "$generator" tests/framework/generate.c > "$directory/generator-build.log" 2>&1; then
    echo "Framework generator build failed: $directory/generator-build.log" >&2; return 2
  fi
  if ! TEST_H_MODE=lifecycle "$generator" > "$directory/runner.c" 2> "$directory/generator.log" ||
     ! compile "$runner" -DNDEBUG "$directory/runner.c" > "$directory/runner-build.log" 2>&1; then
    echo "Framework runner generation/build failed: $directory" >&2; return 2
  fi
  for scenario in success suite-setup suite-teardown setup assertion teardown cleanup preexisting file-ok file-text file-bytes suffix-text suffix-bytes files directories missing capacity command-ok command-exit command-failed-exit command-text command-bytes command-error; do
    local evidence="$directory/$scenario"
    mkdir "$evidence"
    if [[ "$scenario" == preexisting ]]; then
      mkdir "$evidence/fixture"; printf 'unowned\n' > "$evidence/fixture/keep"
    fi
    if TEST_H_SCENARIO="$scenario" ATTENDANT_TEST_DIRECTORY="$evidence" "$runner" -tsv > "$evidence/results.tsv" 2> "$evidence/diagnostics.log"; then
      status=0
    else status=$?; fi
    expected=1; rows=3; failures=1; completed=1; first_failed=1; suite_failed=1
    case "$scenario" in
      success) expected=0; failures=0; first_failed=0; suite_failed=0 ;;
      suite-setup) expected=2; rows=0; failures=0; completed=0 ;;
      suite-teardown) expected=2; failures=0; completed=0; first_failed=0; suite_failed=0 ;;
      teardown|cleanup) first_failed=0 ;;
    esac
    if [[ "$status" -ne "$expected" ]] || ! framework_check_tsv "$evidence/results.tsv" 3 "$rows" "$failures" "$completed"; then
      echo "Framework exit/TSV check failed: $evidence (exit $status, expected $expected)" >&2; return 1
    fi
    {
      printf 'event:suite-setup\n'
      if [[ "$scenario" != suite-setup ]]; then
        for name in first middle last; do
          printf 'event:setup:%s\n' "$name"
          if [[ "$name" != first || ( "$scenario" != setup && "$scenario" != preexisting ) ]]; then printf 'event:body:%s\n' "$name"; fi
          if [[ "$name" == first ]]; then printf 'event:teardown:%s:%s\n' "$name" "$first_failed"
          else printf 'event:teardown:%s:0\n' "$name"; fi
        done
      fi
      printf 'event:suite-teardown:%s\n' "$suite_failed"
    } > "$evidence/expected-events.log"
    grep '^event:' "$evidence/diagnostics.log" > "$evidence/events.log"
    if ! diff -u "$evidence/expected-events.log" "$evidence/events.log" > "$evidence/events.diff"; then
      echo "Framework lifecycle order check failed: $evidence/events.diff" >&2; return 1
    fi
    case "$scenario" in
      success|suite-teardown|suite-setup)
        if [[ -e "$evidence/fixture" ]]; then echo "Unexpected retained fixture: $evidence" >&2; return 1; fi ;;
      preexisting)
        if [[ $(< "$evidence/fixture/keep") != unowned ]]; then echo "Pre-existing fixture changed: $evidence" >&2; return 1; fi ;;
      *) if [[ ! -f "$evidence/fixture/actual" ]]; then echo "Failed fixture not retained: $evidence" >&2; return 1; fi ;;
    esac
    local message=''
    case "$scenario" in
      file-ok) message='destination already exists' ;;
      file-text|file-bytes|suffix-text|suffix-bytes|files|directories) message='contents or layout differ' ;;
      missing) message='file not found' ;;
      capacity|cleanup) message='file operation capacity exhausted' ;;
      command-ok) message='command deadline exceeded' ;;
      command-exit|command-failed-exit) message='expected command exit 0' ;;
      command-text|command-bytes) message='command stdout differs or failed' ;;
      command-error) message='command stderr differs or failed' ;;
    esac
    if [[ -n "$message" ]] && ! grep -q "$message" "$evidence/diagnostics.log"; then
      echo "Missing assertion diagnostic: $evidence" >&2; return 1
    fi
  done
  for scenario in plain empty; do
    local evidence="$directory/$scenario" planned=1 generation_expected=0
    mkdir "$evidence"
    if [[ "$scenario" == empty ]]; then planned=0; generation_expected=3; fi
    if TEST_H_MODE="$scenario" "$generator" > "$evidence/runner.c" 2> "$evidence/generator.log"; then status=0; else status=$?; fi
    if [[ "$status" -ne "$generation_expected" ]] ||
       ! compile "$evidence/runner$executable_suffix" -DNDEBUG "$evidence/runner.c" > "$evidence/build.log" 2>&1 ||
       ! "$evidence/runner$executable_suffix" -tsv > "$evidence/results.tsv" 2> "$evidence/diagnostics.log" ||
       ! framework_check_tsv "$evidence/results.tsv" "$planned" "$planned" 0 1; then
      echo "Framework default/empty suite check failed: $evidence" >&2; return 1
    fi
  done
  local evidence="$directory/standalone" status=0
  mkdir "$evidence"
  # Deliberately omit native flags/sources/libraries: exercise copied header pair.
  if [[ "$compiler" == cjit ]]; then
    "${CJIT:-cjit}" -q -C '' ${cjit_platform_flags[@]+"${cjit_platform_flags[@]}"} -DNDEBUG -Isrc/include tests/framework/command-standalone.c -o "$evidence/runner$executable_suffix" > "$evidence/build.log" 2>&1 || status=$?
  else
    "${ZIG:-zig}" cc -O3 ${zig_platform_flags[@]+"${zig_platform_flags[@]}"} -std=c99 -DNDEBUG -Isrc/include tests/framework/command-standalone.c -o "$evidence/runner$executable_suffix" > "$evidence/build.log" 2>&1 || status=$?
  fi
  if [[ "$status" -eq 0 && $(uname -s) == Darwin ]]; then codesign -s - "$evidence/runner$executable_suffix" >> "$evidence/build.log" 2>&1 || status=$?; fi
  if [[ "$status" -ne 0 ]]; then echo "Standalone header build failed: $evidence/build.log" >&2; return 2; fi
  if ! TEST_COMMAND_HELPER="$ATTENDANT_TEST_COMMAND_HELPER" "$evidence/runner$executable_suffix" > "$evidence/stdout.log" 2> "$evidence/diagnostics.log"; then
    echo "Standalone command check failed: $evidence/diagnostics.log" >&2; return 1
  fi
  echo 'Framework: 26 lifecycle/assertion/default/standalone scenarios passed' >&2
}
