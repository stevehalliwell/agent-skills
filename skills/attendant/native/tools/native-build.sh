# Source from Bash entry points after changing to native root.
# Builds fresh helper using installed compiler; no acquisition or cache adoption.
native_build() {
  local source=$1 compiler=${ATTENDANT_TOOL_COMPILER:-cjit}
  if [[ "$compiler" != cjit && "$compiler" != zig ]]; then echo 'ATTENDANT_TOOL_COMPILER must be cjit or zig' >&2; return 2; fi
  mkdir -p build/native-tools
  export ATTENDANT_TOOL_DIRECTORY
  ATTENDANT_TOOL_DIRECTORY=$(mktemp -d build/native-tools/run-XXXXXX)
  source tools/build-inputs.sh
  source tools/build-platform.sh
  export ATTENDANT_TOOL_BASH
  if [[ $(uname -s) == Darwin ]]; then ATTENDANT_TOOL_BASH=$BASH;
  else ATTENDANT_TOOL_BASH=$(cygpath -m "$BASH"); fi
  flags[1]=-DLIBPDJSON5_STACK_MAX=16
  local output="$ATTENDANT_TOOL_DIRECTORY/tool$executable_suffix"
  if [[ "$compiler" == cjit ]]; then
    if ! "${CJIT:-cjit}" -q -C '' ${cjit_platform_flags[@]+"${cjit_platform_flags[@]}"} "${flags[@]:1}" -Isrc/include "$source" "${sources[@]:1}" -o "$output" ${native_libraries[@]+"${native_libraries[@]}"} > "$ATTENDANT_TOOL_DIRECTORY/build.log" 2>&1; then
      echo "Native helper build failed: $ATTENDANT_TOOL_DIRECTORY/build.log" >&2; return 2
    fi
  else
    if ! "${ZIG:-zig}" cc -O3 ${zig_platform_flags[@]+"${zig_platform_flags[@]}"} "${flags[@]}" -Isrc/include "$source" "${sources[@]:1}" -o "$output" ${native_libraries[@]+"${native_libraries[@]}"} > "$ATTENDANT_TOOL_DIRECTORY/build.log" 2>&1; then
      echo "Native helper build failed: $ATTENDANT_TOOL_DIRECTORY/build.log" >&2; return 2
    fi
  fi
  if [[ $(uname -s) == Darwin ]]; then codesign -s - "$output" || return $?; fi
  NATIVE_HELPER="$output"
}
