# Native C tests

Run from native root:

```bash
bash test.sh cjit
bash test.sh zig
```

Each command runs 56 cases under UTC and Pacific/Auckland, 26 isolated framework scenarios, then five integration cases with 23 contract groups. Builds selected compiler's CLI/API probe automatically. Requires installed compiler/Bash and repository checkout with sibling `skills/init-project/templates/`; no Node, GUI or downloads. Stdout contains retained run-directory path. TSV, generated C, build logs and diagnostics stay under `build/native-tests/run-*/`.

Source/API/core tests live in `core/c/`, subprocess contracts in `core/integration/`. Separate generators share `src/include/test.h` with project-local preambles. Four core JS harnesses were removed after migration; real CLI/API interoperability, stdin/output, deadlines, workflows and isolated cold builds remain tested. All ten shared GUI groups per compiler also use C, including live previews. macOS control/bridge orchestration stays external.

Usability coverage: `core/c/test-navigation.c` checks bounded discovery, owned-link exclusions, tree ordering/expansion and literal matching without GUI library. `gui/usability.h` drives 34 assertions through real widgets on owned fixtures: wrapped Go, source undo/focus, preview provenance, hidden/Clear filter behavior, real divider drags, dirty Close All/Cancel, modal search refusal and rejected clipboard input. Shared live GUI acceptance also sends cross-compiler `--find` controls, waits for bounded applied events, verifies source/preview selection through inspection, checks zero/empty matches and invalid/missing-recipient errors, and compares source snapshots. Clipboard getter is process-local mock; OS clipboard stays untouched. Windows CJIT/Zig pass; new macOS usability checks remain unrun.

## Header boundary

Both headers are reusable C99 project support, not Attendant infrastructure. `base.h` owns ordinary application operations. `test.h` owns assertions, lifecycle and test-owned resources. Attendant sessions, input data, fault names and viewer protocols stay project-local. Neither header contains Attendant paths or environment variables. Do not add test-only process supervision to `base.h` merely to replace JS harnesses.

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
file_ends_with_bytes(path, suffix, size, &equal);
file_ends_with_text(path, suffix, &equal);
file_set_read_only(path, read_only);
file_remove(path);
```

Creation is exclusive. Existing regular file/directory returns `file_error_exists`; refused links return `file_error_unsupported`. No parent creation or overwrite. Text excludes terminating NUL; byte operations support embedded NULs. Empty byte span permits NULL data. Text pointer must not be NULL. Partial new file may remain after write/close failure. Existing `file_write_bytes`/`file_write_all` retain overwrite semantics.

Suffix comparison seeks to tail and reads chunks with fixed 8 KiB scratch; empty suffix matches any regular file. No 32 KiB whole-file ceiling. Read-only setting refuses directories/links. Windows preserves other attribute flags; POSIX clears all write bits for read-only and adds owner write for writable, preserving other mode bits. It does not restore previously removed group/other write bits.

New creation/comparison uses native streams; full-file comparison uses fixed 16 KiB stack scratch. Windows native streams, existing stdio read/write helpers and `file_stat` accept UTF-8 paths. Existing stdio helpers retain ordinary buffering/size limits; new operations do not make writes atomic or crash-durable.

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

`file_directory_list_tree(&files, root, &tree)` exposes bounded root-relative entries/kinds/depths. Result and traversal scratch remain in caller arena until caller rewinds it. Errors rewind temporary storage and leave result unchanged. Copy/compare/removal rewind their own temporary allocations on return. Listing uses the same walker as filtered search; inventory remains strict and unfiltered.

### Filtered recursive search

```c
file_search_options options = {
    .pattern = "*.md",
    .max_results = 4096,
    .max_directories = 262144,
    .max_depth = 32,
    .skip_dot_names = 1,
    .skip_unsupported = 1,
    .issue = on_issue
};
file_error error = file_directory_search(scratch, root, &options, on_match, user);
```

- `on_match`: `file_error (void *user, const file_tree_entry *entry)`. Receives root-relative UTF-8 path with `/` separators, kind and entry depth. Pattern selects regular-file basenames, not directory traversal. NULL pattern includes files and directories. Visitor error stops search after handle cleanup.
- Glob supports `*` sequences and `?` single UTF-8 characters. ASCII case-insensitive by default; `match_case = 1` makes comparison exact. Other Unicode stays exact. Pattern allows 1023 UTF-8 bytes, no path separators. No bracket expressions, DOS wildcard rules or Unicode folding.
- `max_results` counts only entries delivered to visitor and must be positive. A nonmatching regular file consumes no result/directory capacity. `max_directories` separately bounds attempted directory descents below root; 0 means unlimited. Excluded names/links do not consume either budget. Exact limits succeed when no further eligible result/directory remains.
- `max_depth` bounds directory levels below root; 0 permits root files but reports attempted deeper discovery as depth exhaustion. `root_only = 1` instead selects deliberate nonrecursive scope without depth warnings. Filtered root-only search never opens a directory-discovery pass; unfiltered root-only search returns immediate directory entries without entering them. Files in deepest permitted recursive directory remain eligible. Filtered search visits files before subdirectories at each depth, keeping root matches first. Source records remain caller's concern.
- Windows passes masks such as `*.md` to `FindFirstFileW`; separate `FindFirstFileExW` directory pass checks cached attributes even when directory-only filter is ignored. Native `?` counts UTF-16 units, so those masks broaden to `*`; shared UTF-8 matcher preserves character semantics. Postfilter also rejects DOS/short-name extras and applies requested case mode. POSIX enumerates/filter names through same walker. Unrelated names may still require enumeration, but no longer truncate results.
- Dot names and unsupported entries are excluded only when requested. Links/junctions/reparse points are never followed; linked root/ancestors fail even with `skip_unsupported`.
- `on_issue`: `void (void *user, const char *path, file_search_problem problem)`. Receives affected absolute path and result-limit/directory-limit/depth-limit/path/open/read/close/entry-status problem. With handler, recoverable problems warn and skip; without handler, first problem stops. Result/directory exhaustion always stops. Search returns first error even when earlier visits remain usable; allocation failure takes precedence.
- Search paths allow 4096 UTF-8 bytes including NUL. OS limits still apply; no extended-length Win32 path support added. Existing inventory/copy/compare/removal retain 1024-byte path limit.
- Frames allocate only for reached depths and reuse storage across siblings. Scratch remains in caller arena on success/error; caller marks/rewinds it. Callbacks must copy retained paths, never modify borrowed paths, rewind search scratch or longjmp. All handles close before return, including visitor stop and allocation failure while descending.

Attendant temporarily uses `*.md` with `root_only = 1`: root Markdown plus configured collection files, 4096 shared document capacity, no recursive discovery. Project adapter retains deduplication, result storage and warnings. Result paths/warnings allocate before search scratch mark; after handles close, adapter rewinds only temporary frames. No separate arena is needed because visitor never allocates from scratch. Flat collection readers also use shared `file_directory` iterator. No duplicate application walker/platform iterator or compiler-cache exclusion.

## Environment operations in `base.h`

```c
environment_set(scratch, name, value); /* NULL unsets; "" stays present. */
environment_get(scratch, name, output, capacity, &size);
```

Return `environment_error_ok`, `invalid`, `not_found`, `capacity` or `native` (each with `environment_error_` prefix). Caller supplies non-NULL scratch arena; operations rewind it before return. Get capacity includes NUL; size excludes NUL. Errors leave output/size unchanged. Missing and present-empty variables are distinct. Names must be nonempty and cannot contain `=`.

Windows uses validated UTF-8/UTF-16 conversion and OS environment APIs. Scratch exhaustion fails explicitly; no fixed giant stack buffers. POSIX uses byte strings and ordinary libc environment storage. Writes affect current process only. Do not mutate environment concurrently with other readers/writers. Windows CRT `getenv` may cache prior values; pair setter with `environment_get`, not CRT `getenv`.

## Assertions in `test.h`

Arguments evaluate once. Assertions stay active under `NDEBUG` and return `1` from current case/hook on failure. Isolated framework runners compile with `-DNDEBUG` to verify this contract.

```c
TEST_ASSERT_FILE_OK(file_create_text(path, "Hello caf\xc3\xa9.\r\n"));
TEST_ASSERT_FILE_TEXT(path, "Hello caf\xc3\xa9.\r\n");
TEST_ASSERT_FILE_BYTES(path, expected_bytes, expected_size);
TEST_ASSERT_FILE_ENDS_WITH_TEXT(path, expected_suffix);
TEST_ASSERT_FILE_ENDS_WITH_BYTES(path, suffix_bytes, suffix_size);
TEST_ASSERT_FILES_EQUAL(actual_file, expected_file);
TEST_ASSERT_DIRECTORIES_EQUAL(&files, actual_directory, expected_directory);
```

Operation assertion reports source location, operation expression and error. Comparisons report source location, paths/expected-data category, and difference or I/O error. Content/layout differences, missing files and capacity failures remain distinct. Diagnostics do not dump complete contents or report first differing byte/entry. Mutations remain explicit `base.h` calls.

## Test commands in `test.h`

Opt-in test supervision; existing application `base.h` `process_run` is unchanged:

```c
u8 scratch_memory[262144];
char output[65536], error_output[65536];
test_command_workspace commands = {
    .scratch = arena_from_buffer(scratch_memory, sizeof(scratch_memory)),
    .output = output, .output_capacity = sizeof(output),
    .error_output = error_output, .error_capacity = sizeof(error_output)
};
const char *arguments[] = {"my-tool", "--check"};
test_command command = {
    .arguments = arguments, .argument_count = 2,
    .timeout_ms = 30000
};
test_command_result result;
TEST_ASSERT_COMMAND_OK(test_command_run(&commands, &command, &result));
TEST_ASSERT_COMMAND_EXIT(&result, 0);
TEST_ASSERT_COMMAND_OUTPUT_TEXT(&result, "ok\n");
TEST_ASSERT_COMMAND_ERROR_TEXT(&result, "");
```

- Supply positive timeout and valid NUL-terminated argv; count includes executable, at most 192. No implicit shell. Launch Bash explicitly when testing build script. Bare executable searches effective child `PATH`; relative paths/components use child cwd. First regular candidate wins; loader/permission failure does not retry later candidates. This is not shell/`execvp` fallback policy. Windows tries exact name, `.exe`, `.com` when name lacks extension; rejects drive-relative and device/extended namespace forms. Supports normal drive-rooted paths. UNC/network-share paths are outside supported contract; add namespace handling and Windows network-share runtime tests before supporting them. Paths use 1024-byte bound; Windows quoted command uses 32768-byte bound. OS limits still apply.
- Optional `.working_directory` inherits when NULL. Optional `.environment` contains `test_command_environment {name, value}` overrides; NULL value unsets, empty value stays present. Duplicate names fail (case-insensitive ordinal comparison on Windows). At most 4096 inherited-plus-override entries before filtering. Parent cwd/environment/signal policy stays unchanged. Coordinate concurrent changes externally.
- Optional `.input`/`.input_size` supplies raw finite bytes. Empty input gives EOF. Input is spooled to private OS temporary file, not interactive pipe; supplied stdin is seekable. OS/libc handles use ordinary internal storage. No new application heap allocator.
- Captures stay separate and byte-exact, including embedded NUL. Capacity includes convenience terminator; result `.output`/`.error_output` lengths exclude it. Captures borrow workspace buffers until reuse. Input/capture buffers must not overlap; overlapping spans return `test_command_invalid`. Keep command/workspace/result objects and argv/cwd/environment strings outside capture buffers. Scratch allocations rewind to entry mark; pre-existing allocations survive.
- Return `test_command_ok` for completed child, including nonzero exit or POSIX signal. Check exit separately. Windows preserves raw unsigned native exit/exception status in `.exit_code`; POSIX reports ordinary exit through `.exited`/`.exit_code` or termination through `.signal_number`. Timeout is not a fabricated expected exit.
- Invalid configuration, launch, I/O, timeout, capacity, cancellation and cleanup errors have distinct enum values/messages. Result records error/native OS code, partial captures and `.cleanup_failed`. Capture overflow cancels child; it never silently truncates into success. Exit/output assertions require operation success, so timeout followed by exit zero cannot pass. Text assertions compare byte lengths, not NUL-prefix alone.
- Windows starts suspended with only three standard handles inherited, assigns owned kill-on-close job, then resumes. Completion/cancellation requests termination of remaining members, waits direct child and verifies job active count reaches zero. Job-empty does not guarantee every descendant process object is already signaled; callers needing that observation must separately wait known descendant handles. POSIX creates owned group, cancels original group and direct PID before reaping; `WNOWAIT` preserves PID ownership until cancellation. Detached POSIX descendants remain outside contract; grandchildren can become init-owned zombies. This is not a process sandbox.
- Darwin may return `EPERM` when group signaling finds only exiting processes/zombies. Accept it only after OS inventory confirms no live members; inventory is bounded to 64 members. Query/capacity failures and live members retain cleanup failure. Child stays unreaped until group cancellation completes.
- POSIX requires default `SIGCHLD`, no competing reaper, monotonic clock and modern libc's spawn chdir action. Enable platform feature macros before system headers (for example, `-D_GNU_SOURCE` on Linux with strict C99). Darwin closes unrelated inherited descriptors; other POSIX callers must use close-on-exec discipline. Windows needs modern job/extended-startup APIs. Linux is compile-checked only, not a product runtime target.
- Deadline covers preparation/launch/capture. Cleanup adds bounded waits: up to 1000 ms for direct child, plus Windows job accounting wait up to 1000 ms. OS calls/scheduling are not hard real-time guarantees. Cleanup errors fail rather than report success; uninterruptible OS work can outlive cancellation. Parent crash/forced termination can bypass cleanup; Windows kill-on-close helps after assignment. No parent crash isolation, asynchronous lifetime manager or parallel scheduler.

Additional assertions: `TEST_ASSERT_COMMAND_OUTPUT_BYTES`, `TEST_ASSERT_COMMAND_ERROR_BYTES`, `TEST_ASSERT_COMMAND_ERROR_TEXT`. Arguments evaluate once; all stay active under `NDEBUG`. JSON decoding/reporting and application readiness protocols stay project-local.

### Caller-driven live commands

`test_command_run` wraps same start/poll/finish lifecycle. No worker thread, background watchdog or scheduler:

```c
/* Stable state; case teardown must finish both, including after assertion return. */
static test_command_live viewer, inspector;
TEST_ASSERT_COMMAND_OK(test_command_start(&viewer, &viewer_workspace, &viewer_command));
/* Poll viewer until project-local complete readiness event. */
TEST_ASSERT_COMMAND_OK(test_command_start(&inspector, &inspector_workspace, &inspect_command));
while (!inspector.complete) {
    TEST_ASSERT_COMMAND_OK(test_command_poll(&viewer));
    TEST_ASSERT_COMMAND_OK(test_command_poll(&inspector));
    /* Inspect bounded captures; yield on idle polls. */
}
/* In teardown, before assertions or fixture removal: */
test_command_error viewer_error = test_command_finish(&viewer);
test_command_error inspector_error = test_command_finish(&inspector);
```

- Zero-initialize before first start. Never copy/move active state. Separate capture buffers remain stable and exclusively reserved through completion. `.result` contains current bounded capture spans/error; `.active` means resources remain owned, `.complete` means terminal cleanup attempt finished (check `.cleanup_failed`), `.observed` means direct-child exit was observed (captures may still drain).
- Start borrows argv/input/environment/cwd and scratch only until return. Scratch rewinds immediately and can serve another start. Retained workspace copy uses only captures during polling. Failed start completes with no owned live resources; restarting active state returns invalid without changing ownership/result. Completed state may be reused.
- Poll never waits for readiness or child progress. It drains at most 16 KiB per stream, checks original monotonic deadline before/after backend work and performs bounded terminal cleanup on completion/error. Poll every live child; blocking caller work delays deadline enforcement. Required timeout still covers preparation/launch/capture, but no hard background deadline promise.
- Polling completed state returns prior result; polling unused state returns invalid. Finish first polls active state, then cancels if still active, returning `test_command_cancelled`. Finish on unused state returns success; repeated finish preserves result. Exit/signal remains separate from infrastructure result. Cleanup failures remain visible even after another error.
- Local stack-owned states need explicit cleanup before any assertion/early return. Hook-managed states and captures must outlive body return. Finish all children before asserting cleanup results. Crashes still bypass hooks.

## Owned directories in `test.h`

Opt-in helper; no implicit paths, environment variables, allocation or registry:

```c
test_directory fixture = {0};
TEST_ASSERT_FILE_OK(test_directory_create(&fixture, path));
char baseline[NATIVE_PATH_SIZE];
TEST_ASSERT_FILE_OK(test_directory_next(&fixture, "baseline", baseline));
TEST_ASSERT_FILE_OK(file_directory_copy(&files, actual_directory, (cstring){baseline}));
/* In teardown, including after partial setup or assertion return: */
TEST_ASSERT_FILE_OK(test_directory_finish(&fixture, &files, failed));
```

- Create requires a missing directory path with existing, non-link ancestors. Ownership begins only after exclusive creation. Existing files/directories/links are never adopted or removed.
- Zero-initialize descriptor. Do not copy ownership or change its path while owned. Caller owns memory/lifetime and invokes finish from hooks.
- Next reserves numbered child path, not file/directory. Label is one nonempty component, not `.`/`..` and without slash/backslash/colon. Numbering starts at 1. No collision retries. Output must not overlap descriptor. Counter/path exhaustion fails explicitly. Create/copy reserved destination exclusively.
- Keep copied tree outside its own baseline destination; for example, actual and baseline can be sibling children of fixture.
- Finish removes successful owned directory or retains/reports failed one. Cleanup failure retains/reports path and returns error; propagate it to fail hook/case.
- Every finish attempt ends ownership. Later finish is harmless and cannot delete retained evidence. Descriptor can create a different new directory for next case; retained paths remain available for inspection until descriptor reuse. Creating same retained path fails rather than adopting it.
- Parent cleanup invalidates nested child descriptors; do not finish those afterward. No automatic destructors, retries or crash recovery.

## Attendant fixture policy

`core/c/support.h` binds one reused API session, sample data, feedback requests, fault names and scratch workspace. Shared directory helper owns suite/case policy; general helpers handle environment, attributes and suffix checks. Compact numbered case labels include Unicode; TSV preserves full names without extending legacy Win32 paths. Baselines use shared child naming and independent directory copies, not implementation-produced hashes.

Fault injection clears before/after each case. Session closes in suite teardown. Wrapper supplies owned run directory; local setup validates it below `build/native-tests`. Core inventories use 64 KiB scratch, depth 12 and 128 entries per tree. Integration inventories use 4 MiB, depth 32 and 8192 entries. Commands use 256 KiB scratch and separate 1 MiB captures plus terminators, matching prior Node capture limits. Project-local JSON uses two 2 MiB arenas, 4096 nodes and depth 16; only integration parser build raises JSON stack bound to 16. Product CLI parser bound stays 4.

Integration preserves 10 CLI, eight API/CLI, four workflow and one install contract groups per compiler. Actual CLI/API-probe children remain separate compiled consumers. Independent expected hash covers saved feedback fixture; snapshots compare exact shared directory copies, including empty directories. Cache/build exclusions stay in project-local one-level copy policy, not reusable headers. Each command saves stdout/stderr below owned case folder. Successful folders are removed; failures retain captures/snapshots. Install case owns separate short OS temp path with spaces/Unicode, excludes native builds, unsets inherited Zig cache paths and cold-builds copied source. Teardown removes successful installation or reports retained path; never adopts existing directory.

## Shared GUI acceptance

After both compiler CLI/production GUI/check GUI builds:

```bash
bash tools/check-gui.sh
```

Coordinator verifies both compilers' build/stack/binary manifests before launching tests. It compiles fresh per-compiler runner with `tools/build-gui-tests.sh`; generated C lives in `build/gui-tests/run-*/`. `tests/gui/integration/` owns separate hooks and exactly ten groups, not core integration's 23-group policy. Shared bounded JSON lives in project-local `tests/support/integration-json.h`, included in GUI manifest inputs. Parser stack 16 applies only to test runners, not product builds.

Eight finite groups: production render/P6, existing C model, terminal renderer failure, partial discovery, navigation/tab reuse, undo isolation, edit/feedback resolution and saved-not-queued preservation. Existing model assertions run in separate executable, not duplicated in parent. Two live-preview groups/compiler now run in C: complete JSON-line `selection-ready`, opposite-compiler inspection while viewer stays live, scalar UTF-8 context/range checks, dirty state, saved bytes/independent expected hash, prior-file preservation, Save & Send and explicit resolution/body preservation. Poll viewer and inspector together. Native `tests/gui/coordinator.c` owns provenance/build/result validation. Total stays 20 groups across both compilers. Separate native macOS socket/control/bridge coordinators replace their old JS harnesses; macOS compilation/runtime qualification passes with CJIT/Zig; see [current evidence](../../../../TODO.md#checks).

Use `ATTENDANT_GUI_VARIANT` for fresh isolated GUI builds; never overwrite existing viewers' binaries. Coordinator builds C runners automatically and preserves compiler/model counts. It requires exact ten source/name pairs and successful complete TSV, not process exit/count alone; model count must appear exactly once and stay in positive `int` range. Each runner build executes focused validator checks under `NDEBUG`: 12 count cases (including negative/fractional/overflow/missing values), six P6 cases (including invalid magic, truncation, extra bytes and dimensions), five scalar UTF-8 slice cases and three fragmented/readiness cursor cases. Expected rejection diagnostics stay in isolated `validation/` build logs. Compiler timeout is 180000 ms; runner outer timeout 900000 ms. Each normal GUI/model/CLI/inspection child has 60000 ms deadline; terminal-render failure and live preview viewer use 30000 ms. Command scratch/captures and file/JSON budgets match core integration. Each live child has separate 1 MiB stdout/stderr buffers; starts share reusable 256 KiB scratch.

Each compiler gets owned Unicode workspace fixtures under `build/gui-check/run-*/native-<compiler>/fixtures/`. Independent tree copies compare names, empty directories and bytes; saved-not-queued baseline changes only known expected README bytes. Successful fixtures disappear; failures retain command stdout/stderr and snapshots. TSV/diagnostics and `production.ppm` stay in compiler report folder. Screenshot stays outside Unicode fixture path, matching old harness: existing renderer screenshot writer has narrow-path limitation. Live previews now use same owned-case cleanup: teardown finishes both children before writing retained failure captures or removing successful fixtures.

## Native tooling

All nine remaining Attendant JS tools/tests are replaced. Entry points run from native root:

```bash
bash tools/native-tool.sh --help
bash tools/native-tool.sh prepare --verify
bash tools/check-native-tools.sh
ATTENDANT_TOOL_COMPILER=zig bash tools/check-native-tools.sh
bash tools/benchmark.sh 5
# macOS only:
bash tools/check-control-macos.sh
bash tools/check-bridge.sh
```

`native-build.sh` compiles a fresh helper in exclusively created `build/native-tools/run-*/`. Direct entry points default to CJIT; `ATTENDANT_TOOL_COMPILER=zig` selects installed Zig. Stack builds default their helper to required Zig; application/bridge builds default to selected application compiler. Explicit helper override wins. `CJIT`/`ZIG` select executable paths. No download, cache reuse or Node bootstrap.

`tools/native/` is project-local, not reusable framework. Ordinary preparation/patch/manifest commands use unchanged `base.h` `process_run` and explicit Bash capture. Bootstrap records exact Bash path so Windows cannot select WSL's `bash.exe` ahead of Git Bash. Bash opens separate owned stdout/stderr files; after wait, C appends stderr bytes to argv log. This avoids MSYS writes through Win32 append-only handles without changing shared process API. Ordinary build/Git commands block like prior tools, without test deadlines. Capture/text caps apply after command exits; retained files are not disk-size bounded.

Bounds: three 8 MiB text buffers, 8 MiB file-tree scratch, depth 32, 8192 entries/tree, 1200 provenance files, 1024-byte paths, 64 KiB compiler identity buffer, 256 KiB preview-method include. JSON keeps shared two 2 MiB arenas/4096 nodes/depth 16. SHA-256 streams binary bytes through BCrypt/CommonCrypto; never reuse text-only core digest. Inventories are sorted, content hashed, platform/compiler/mode checked, and actual stack/copied library/binaries verified before GUI tests.

Preparation retains exact pins and explicit acquisition boundary. `prepare --verify` never acquires missing checkouts; successful verification still writes `build/dependencies.json`, matching prior behavior. Strict ordered patch data reconstructs expected files from pinned Git inputs, reads authoritative preview include and refuses unknown edits, NUL tails, malformed UTF-8 or changed anchors. Generated fixes require exactly two named typedefs and three exact union anchors; regenerate before rerunning, rather than accepting already-fixed/changed input.

Maintained tooling checks cover ten groups: binary SHA-256 vectors/chunking, quoted Unicode argv/separate stderr/nonzero exit, strict replacement/count/capacity refusal, generated C99 fixes/drift, owned Git source patches/idempotence/offline verification/unknown-edit refusal, exact GUI identities/completion/model markers, bridge exact ABI/lifetime receipts (including embedded-NUL tail rejection) and P6 dimensions/aspect/colors/exact top/bottom markers, provenance tamper/path/list refusal, CLI/missing-offline-input errors, and cold pinned acquisition followed by offline verification. Acquisition test rewrites upstream URLs to local checkouts only and sets `GIT_ALLOW_PROTOCOL=file`; no network fallback. Fixture Git commits stay inside exclusively owned temporary repository, never project history. Success cleanup relaxes Git object read-only flags only in owned fixture; failures retain evidence.

Test/benchmark coordinators use `test.h` command supervision, not ordinary process API. Benchmark requires 1..1000 runs, warms sync/query/search on separate fixture copies, rotates serial compiler order, validates replies outside measured interval and records samples/min/median/max. Child deadline is 30000 ms. `build/bench/run-*/results.json` is synthetic process evidence, not representative performance.

macOS control suite polls all live viewers/clients together, keeps finite raw socket bounds, preserves 16 groups and independent source checks, and closes owned sockets/children before fixture cleanup. Bridge suite runs both real C consumers, validates exact lifetime/ABI JSON and P6 orientation/content, and hashes signed executable/screenshot/input identities into report. Windows executes shared validators, not macOS transports/rendering. macOS follow-up passes; evidence and scoped test fixes live in [project checks](../../../../TODO.md#checks).

## Verification boundaries

Windows CJIT/Zig cover binary/NUL/Unicode/empty/streamed files and suffixes, exact directory names/kinds/content, copy/create exclusivity, capacity/depth/memory failures, roots/cwd guards, junction/ancestor/broken-link refusal, Unicode aliases, read-only cleanup failure, environment empty/unset/UTF-8/capacity behavior and owned-directory reuse/retention. Commands cover Unicode/quotes/backslashes/no shell, binary stdin/EOF, separate saturated pipes, exact/overflow capacities, cwd identity, child environment inheritance/override/unset without parent mutation, missing executable/cwd, nonzero/abrupt termination, timeout and descendants with open/closed standard handles. Controlled readiness follows first flushed heartbeat and handle closure. Live checks cover fragmented readiness, concurrent binary-stdin inspector, scratch reuse, active restart rejection, failed launch/reuse, deadline crossing within poll, overflow, cancellation and timeout with open/closed descendant streams. Windows known-descendant checks use bounded 1000 ms handle waits after command return, then heartbeat checks; job-empty alone is not process-object signaling.

`framework/` checks lifecycle order, skipped bodies, continuation, suite failure/incomplete TSV, partial setup, retention/ownership and no-hook/empty defaults. Standalone binary-stdin command consumer compiles/runs with only C99/include/platform flags, no Attendant sources/defines/libraries. Negative command assertions include timeout with exit zero and embedded-NUL text mismatch. POSIX branches add symlink/FIFO and moved-group direct-child cancellation checks. Generic framework/helpers cross-compile to Linux; Fresh macOS arm64 CJIT/Zig runtime validation passes core/framework/integration, tooling, shared GUI, control and bridge checks. Standalone regression covers Darwin group cleanup permission/state/query/capacity cases. Maximum-inspection model client waits 2000 ms, matching unchanged server deadline; stage and byte-count diagnostics remain on failure. Windows branches were not rerun on macOS. Evidence and remaining limits live in [project checks](../../../../TODO.md#checks).
