# C99 YAML front-matter spike

## Result

LibYAML merits a further bounded prototype: both Windows compiler paths built and ran this probe. Its document emitter discarded comments and changed unrelated formatting; its parser accepted duplicate keys. It is not a drop-in Attendant parser or source-preserving editor. libfyaml remains untested at runtime; its build needs further dependency and build-system work. No library adoption decision follows from this spike.

Parent review independently rebuilt LibYAML at the pinned revision with CJIT and `zig cc`, reran `check.mjs`, and obtained output identical to `results.txt`. Confirmed real fixture and original source hashes match. Review build outputs and clone were removed. libfyaml failure was reviewed from the worker report, not independently reproduced.

Raw observations: `results.txt`. `check.mjs` prints observations rather than asserting every expectation. It uses Node's string/UTF-8 conversions; byte claims cover the valid UTF-8 fixtures, not arbitrary invalid UTF-8. The only assertion in the harness checks accepted splice body text. No source record was written.

## Inputs and provenance

- LibYAML: https://github.com/yaml/libyaml, commit `90a56d4500aa1a1798514c5cb55c3ad4cb095f94`, source version 0.2.5 from `configure.ac`, MIT license (`License`). Probe compiler version macros set to that version. No library files retained.
- libfyaml: https://github.com/pantoniou/libfyaml, commit `1f2ba67561276e666a7e77bf9ff299dab50d3700`, MIT license (`LICENSE`). No library files retained. Revision identifies version; no built library version was observed.
- Both licenses require their copyright and permission notices when distributing copies or substantial portions. Review vendoring obligations, transitive dependencies and attribution before any adoption.
- `fixtures/real.md`: copy of `D:/Repos/proto-framework/records/decisions/vendor-project-dependencies-by-default.md`, repository HEAD `f0b848e2541903ac99696f1f8a71d93e72ef4c5b`. Both files had SHA-256 `f0997a1b0947d9b03a5feba8d0fe72b2a3ee4bb85f2d41cb9477216ec626b531` after testing. The copy includes a sequence, date-time and Markdown body; synthetic inputs in `check.mjs` supply comments, Unicode and other cases.
- Windows x64; CJIT `v1.2.0`, Zig `0.17.0` (`zig cc`: clang 22.1.8, target `x86_64-unknown-windows-gnu`), Node `v24.16.0`. No tools or dependencies installed. Optional framework headers were not needed.

## Reproduce

From Git Bash in this folder, with Git, CJIT, Zig and Node on PATH. Clones and build outputs stay here; do not run commands in a source-record directory. Network is needed for clones. The cloned sources are not retained in this worktree.

```sh
git clone https://github.com/yaml/libyaml.git libyaml
git -C libyaml checkout 90a56d4500aa1a1798514c5cb55c3ad4cb095f94
git clone https://github.com/pantoniou/libfyaml.git libfyaml
git -C libfyaml checkout 1f2ba67561276e666a7e77bf9ff299dab50d3700
cjit -h
zig version
node --version

cjit -DYAML_DECLARE_STATIC -DYAML_VERSION_MAJOR=0 -DYAML_VERSION_MINOR=2 -DYAML_VERSION_PATCH=5 '-DYAML_VERSION_STRING="0.2.5"' -Ilibyaml/include -o probe-cjit.exe probe.c libyaml/src/{api,dumper,emitter,loader,parser,reader,scanner,writer}.c
export ZIG_GLOBAL_CACHE_DIR="$PWD/zig-cache-global"
zig cc -std=c99 -DYAML_DECLARE_STATIC -DYAML_VERSION_MAJOR=0 -DYAML_VERSION_MINOR=2 -DYAML_VERSION_PATCH=5 '-DYAML_VERSION_STRING="0.2.5"' -Ilibyaml/include probe.c libyaml/src/{api,dumper,emitter,loader,parser,reader,scanner,writer}.c -o probe-zig.exe
node check.mjs ./probe-cjit.exe ./probe-zig.exe > results.txt

zig cc -std=c99 -Ilibfyaml/include -Ilibfyaml/src -Ilibfyaml/src/util -Ilibfyaml/src/lib -c libfyaml/src/lib/fy-doc.c -o fy-zig.o
cjit -Ilibfyaml/include -Ilibfyaml/src -Ilibfyaml/src/util -Ilibfyaml/src/lib -c libfyaml/src/lib/fy-doc.c
```

The LibYAML commands passed independently and both executables produced the recorded results. First Zig attempt without a local global cache failed at link: `lld-link: error: could not open ... crt2.obj: No such file or directory`. Retrying with `ZIG_GLOBAL_CACHE_DIR` under the spike passed; cause of the default cache failure was not established. Zig may also use its normal local cache; set `ZIG_LOCAL_CACHE_DIR` to a spike-local path if needed.

The libfyaml commands are exploratory single-file compilation, not full builds. Both stopped at `fatal error: 'xxhash.h' file not found` after adding the source include directories. CMake was not on PATH (`cmake: command not found`); its supported CMake build was not attempted. Missing xxhash is a dependency/include blocker in this direct experiment, not proof that the supported build or compiler cannot work. No libfyaml parsing, comments, emission or Windows executable tested. No automatic dependency fetching or installation performed.

## Current behavior and comparison

`skills/attendant/scripts/attendant.mjs` uses `parseRecord` and `buildText`: opening delimiter must be first and followed by LF or CRLF; closing delimiter is a separate `---` line. YAML document parsing uses `uniqueKeys: true` and requires a mapping. Validation separately requires nonempty string keys and top-level scalar/null or scalar-array values, then applies collection and universal field rules, including ISO date/datetime strings. Nested mappings and alias values fail subset validation. Runner tests in `skills/attendant/test/attendant.test.js` cover create/update, body retention and batch rejection, but do not establish comment or exact formatting preservation.

The comparison harness reproduces the delimiter/document/update operations with the runner's vendored `yaml.mjs`; it is not an invocation of the full CLI or complete collection validation. Its `subset` indicator is an approximation. The current serializer preserves comments in these examples, not a general byte-preservation guarantee. It changed unrelated formatting in the synthetic mixed-value example. LF body suffix was retained on tested inputs. On CRLF inputs the runner-equivalent regex consumes the CR in the closing delimiter, and `buildText` reconstructs `---` plus the remaining LF: the closing line becomes LF even though the YAML lines use CRLF. `front-newline=true` in the raw log checks YAML only; `whole-newline=false` exposes that mixed line ending. Body bytes in the log mean the suffix *after the matched closing delimiter*, not the CR consumed by that match.

## Observed checks

- Both native executables accepted real, scalar/array/Unicode, LF, CRLF and no-final-newline mappings. Node's parser resolved synthetic values as string `old`, boolean `true`, null, number `42`, date and datetime strings, string array and quoted string `true`; see `runner-values`. Native round-trip output, parsed with the runner's YAML library, retained those values after changing `status` and inserting string `spike_added: "yes"`. This does not establish LibYAML's independent type resolution: LibYAML exposes scalar nodes/tags, and the probe does not implement Attendant's schema resolution.
- Both native parsers rejected malformed flow sequence with `did not find expected ',' or ']'`; rejected empty front matter as non-mapping; accepted duplicate keys, nested mappings and aliases. Runner document parser rejects duplicate keys; its validation rejects nested and alias values. Native parser needs explicit duplicate and subset checks. Missing opening or closing delimiters were rejected by the comparison boundary before invoking either native parser; the C probe consumes YAML only.
- Real record edit: semantic comparison, unrelated formatting, newline style and body suffix passed for both native outputs and runner-equivalent output. There are no comments in this front matter, so `comments=true` is vacuous. Insertion and replacement were tested together.
- Simple LF and CRLF edits: semantics and body suffix passed for both native outputs; native comments and unrelated formatting failed. LF newline passed; CRLF whole-record newline failed due to the comparison wrapper's delimiter reconstruction. Runner-equivalent preserved comment and unrelated front formatting; CRLF closing line still changed as above.
- Mixed synthetic LF, CRLF and no-final-newline edits: semantics and body suffix passed; native comments and unrelated formatting failed; runner-equivalent retained comments but changed unrelated formatting. LF and no-final-newline newline checks passed; CRLF whole-record newline failed. The input without final newline had a nonempty body with no final newline. Native emitter output is normalized to the input front-matter newline style by the harness, not automatically by LibYAML.
- `comments` checks whether any comment marker survives, not exact placement or text; `unrelated-format` compares the remaining YAML after removing the changed and inserted lines. `semantic` compares the runner-parsed resulting JS values, including all unchanged values. These are fixture-specific checks, not full compatibility tests.

## Source splice experiment

`check.mjs` also tests a deliberately narrow JS source splice, not a C implementation. It requires a valid unique-key mapping, one plain `status: word` line, no existing inserted field, and rejects indicators for anchors, aliases, tags, flow collections and block scalars. It reparses and checks resulting values before returning text. The simple LF and CRLF cases passed an exact expected-text comparison, including comments, unrelated formatting, newline style and body. Real and mixed examples were rejected because they contain flow sequences; alias, nested, malformed and duplicate cases were rejected. This conservative filter is not a proof of safety for arbitrary YAML: quoting, multiline values, directives, duplicate semantic keys, encoding, offsets and parser/schema differences need a designed grammar and adversarial tests. A C approach could use parser marks to locate tokens, but those marks and UTF-8 offsets were not tested here. Never apply this probe to real records.

## Remaining risks

- LibYAML probe loads only one document and does not enforce end-of-stream, resource bounds, every duplicate key or the runner's full validation. It is not safe as a production editor. Multiple YAML documents, explicit tags, complex keys, aliases, block scalars, encoding errors and resource exhaustion need tests.
- No native source-preserving editor, libfyaml behavior, compiler coverage beyond these versions, or independent native semantic resolver was tested. Libfyaml's comment support and placement remain unknown.
- The existing test suite was inspected, not run. No CLI, Markdown parser, SQLite, source record or parent worktree changes were made. Generated executables, source clones and caches were removed after testing.
