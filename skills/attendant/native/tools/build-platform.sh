# Shared native CLI/API compiler platform inputs. Source from native root.
# No downloads or installs. Windows outputs keep existing names/link behavior.
executable_suffix=.exe
native_libraries=(-lbcrypt -lshell32)
zig_platform_flags=()
cjit_platform_flags=()
case $(uname -s) in
  Darwin)
    sdk=${SDKROOT:-$(xcrun --show-sdk-path)}
    if [[ ! -d "$sdk/usr/include" ]]; then
      echo 'Apple SDK headers unavailable; set SDKROOT or configure installed command-line tools.' >&2
      exit 1
    fi
    executable_suffix=''
    native_libraries=()
    zig_platform_flags=(--sysroot "$sdk" -isystem "$sdk/usr/include")
    cjit_platform_flags=(-I "$sdk/usr/include")
    ;;
  MINGW*|MSYS*|CYGWIN*) ;;
  *) echo 'Native Attendant build supports Windows and macOS only.' >&2; exit 1 ;;
esac
export ZIG_GLOBAL_CACHE_DIR=${ZIG_GLOBAL_CACHE_DIR:-"$PWD/build/zig-global-cache"}
export ZIG_LOCAL_CACHE_DIR=${ZIG_LOCAL_CACHE_DIR:-"$PWD/build/zig-local-cache"}
