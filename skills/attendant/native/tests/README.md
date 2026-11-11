# Native C tests

Run from native root:

```bash
bash test.sh cjit
bash test.sh zig
```

Each command generates/builds runner, runs 35 cases under UTC and Pacific/Auckland, then checks 17 isolated framework scenarios. Requires installed compiler and Bash; no Node, GUI or downloads. Stdout contains retained run-directory path. TSV, generated C, build logs and diagnostics stay under `build/native-tests/run-*/`.

Source/API/core tests live in `core/c/`. `core/generate.c` uses shared `src/include/test.h` generator with Attendant fixture preamble. External CLI/API/workflow/install/GUI harnesses remain separate; they preserve subprocess deadlines, stdin/output and desktop behavior.

## Lifecycle

Existing `TEST(name, ...)`, `TEST_ASSERT`, `TEST_PASS` and `TEST_FAIL` remain valid. Bind optional callbacks in generated preamble or included suite file:

```c
static int suite_setup(void);
static int suite_teardown(int failed);
static int case_setup(const test_case *test);
static int case_teardown(const test_case *test, int failed);

#define TEST_SUITE_SETUP suite_setup
#define TEST_SUITE_TEARDOWN suite_teardown
#define TEST_SETUP case_setup
#define TEST_TEARDOWN case_teardown
```

All callbacks return zero on success. Missing callbacks are no-ops. One generated executable is one suite; hooks are not per-source-file groups. `test_case.source` and `.name` are borrowed runner literals; metadata remains valid through case teardown. Keep fixture state typed and caller-owned. Runner is sequential; no thread-local or untyped fixture context.

Order:
1. Attempt suite setup.
2. For each case, attempt setup; run body only on setup success; always attempt case teardown.
3. Always attempt suite teardown after suite setup attempt, including failed setup.

Teardown must tolerate partial setup. `failed` reflects earlier failures, not callback's own eventual result. Case teardown failure fails case. Suite teardown receives suite setup, case and result-row output failures. Early assertion returns do not skip teardown. Crashes, `abort`, `exit` and forced termination do skip teardown; no signal/exception recovery added.

Exit contract:
- `0`: complete passing suite.
- `1`: complete suite with failed cases, including case setup/teardown failure.
- `2`: suite hook or output failure; no successful completion record.

TSV schema stays `kind/status/source/name`. `suite` gives planned cases; each `test` row gives `ok` or `fail`; `complete` gives failed, handled and passed case counts. Failed case setup still produces failed case row; its body did not run. Suite hook failure omits `complete`. Generator retains exit `3` for empty discovery and emits valid zero-case runner; `test.sh` treats unexpected empty main suite as generation failure.

## Filesystem operations in `base.h`

Paths use existing `cstring` type. New operations return `file_error`; comparison differences return `file_error_ok` with `equal == 0`, not an I/O error. Comparison outputs stay unchanged on errors.

```c
file_create_bytes(path, bytes, size);
file_create_text(path, text);
file_equal_bytes(path, expected, size, &equal);
file_equal_text(path, expected, &equal);
file_equal(left, right, &equal);
file_remove(path);
```

Creation is exclusive. Existing regular file/directory returns `file_error_exists`; refused links return `file_error_unsupported`. No parent creation or overwrite. Text excludes terminating NUL; byte operations support embedded NULs. Empty byte span permits NULL data. Text pointer must not be NULL. Partial new file may remain after write/close failure. Existing `file_write_bytes`/`file_write_all` retain overwrite semantics.

New creation/comparison uses native streams; file comparison uses fixed 16 KiB stack scratch. Windows native streams, existing stdio read/write helpers and `file_stat` accept UTF-8 paths. Existing stdio helpers retain ordinary buffering/size limits; new operations do not make writes atomic or crash-durable.

Tree operations use explicit scratch/limits:

```c
u8 memory[65536];
file_tree_workspace files = {
    .memory = arena_from_buffer(memory, sizeof(memory)),
    .max_depth = 12,
    .max_entries = 128
};

file_directory_copy(&files, source, new_destination);
file_directory_equal(&files, actual, expected, &equal);
file_directory_remove_tree(&files, owned_directory);
```

- Root depth is 0. Limits must be positive. Entry limit excludes root and applies separately to each tree. Empty roots are valid.
- Compare exact relative names, kinds, empty directories and file bytes. Ignore metadata. No newline, case or Unicode normalization.
- Copy requires missing destination outside source. Never merge. Native copy may preserve metadata; comparison does not depend on it.
- Removal includes root, refuses filesystem root/cwd/cwd ancestors, and never clears read-only flags. Missing root returns `file_error_not_found`.
- Validate root ancestors and inventory before mutation. Reject links/reparse points, broken links and special entries without following them.
- Iterative traversal and in-place heapsort avoid recursive traversal/new application heap allocation. Existing OS/libc handles retain ordinary internal storage. Inventory scratch belongs to caller. Entry/depth/path exhaustion returns `file_error_capacity`; exhausted arena returns `file_error_out_of_memory`.
- Copy/removal can leave partial effects after mutation starts. Trees must be externally quiet. These checks are not an adversarial filesystem sandbox.
- Paths are bounded by `NATIVE_PATH_SIZE` (1024 UTF-8 bytes including NUL). OS limits also apply; extended-length Win32 paths are not enabled.

`file_directory_list_tree(&files, root, &tree)` exposes bounded root-relative entries/kinds/depths. Result remains in caller arena until caller rewinds it. Errors rewind temporary storage and leave result unchanged. Copy/compare/removal rewind their own temporary allocations on return.

## Assertions in `test.h`

Arguments evaluate once. Assertions stay active under `NDEBUG` and return `1` from current case/hook on failure. Isolated framework runners compile with `-DNDEBUG` to verify this contract.

```c
TEST_ASSERT_FILE_OK(file_create_text(path, "Hello caf\xc3\xa9.\r\n"));
TEST_ASSERT_FILE_TEXT(path, "Hello caf\xc3\xa9.\r\n");
TEST_ASSERT_FILE_BYTES(path, expected_bytes, expected_size);
TEST_ASSERT_FILES_EQUAL(actual_file, expected_file);
TEST_ASSERT_DIRECTORIES_EQUAL(&files, actual_directory, expected_directory);
```

Operation assertion reports source location, operation expression and error. Comparisons report source location, paths/expected-data category, and difference or I/O error. Content/layout differences, missing files and capacity failures remain distinct. Diagnostics do not dump complete contents or report first differing byte/entry. Mutations remain explicit `base.h` calls.

## Attendant fixture policy

`core/c/support.h` owns one reused API session and exclusively creates suite/case directories below wrapper-supplied run directory. Compact numbered case directories include Unicode; TSV preserves full case names without extending legacy Win32 paths. Baselines are independent directory copies, not hashes produced by implementation under test.

Successful cases delete owned directories. Failed cases retain/report paths. Cleanup failure fails case and reports remaining path. Never adopt/remove pre-existing fixture directories. Fault injection clears before/after each case. Session closes in suite teardown. Working inventories use 64 KiB scratch, depth 12 and 128 entries per tree. Local Markdown suffix checks retain 32 KiB read capacity.

## Verification boundaries

Windows CJIT/Zig cover binary/NUL/Unicode/empty/streamed files, exact directory names/kinds/content, copy/create exclusivity, capacity/depth/memory failures, roots/cwd guards, junction/ancestor/broken-link refusal, Unicode case aliases and read-only cleanup failure. `framework/` checks lifecycle order, skipped bodies, continuation after case failure, suite failure exits/incomplete TSV, assertions, partial setup cleanup, failure retention, pre-existing ownership and no-hook/empty defaults.

POSIX branch has symlink and FIFO checks. macOS runtime validation of these extensions remains unrun on Windows host. Process capture/deadlines, crash isolation and parallel suites remain outside scope.
