# Shared C99 application/test inputs.
# Source from native product directory; do not execute directly.
sources=(apps/cli/main.c vendor/libpdjson5/pdjson5.c vendor/sqlite/sqlite3.c vendor/libyaml/src/{api,dumper,emitter,loader,parser,reader,scanner,writer}.c)
flags=(-std=c99 -DLIBPDJSON5_STACK_MAX=4 -DSQLITE_ENABLE_FTS5 -DSQLITE_DISABLE_INTRINSIC -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -DYAML_DECLARE_STATIC -DYAML_VERSION_MAJOR=0 -DYAML_VERSION_MINOR=2 -DYAML_VERSION_PATCH=5 '-DYAML_VERSION_STRING="0.2.5"' -Ivendor/sqlite -Ivendor/libyaml/include -Isrc -Isrc/core -I.)
