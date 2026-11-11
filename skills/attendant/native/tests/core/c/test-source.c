#include "test.h"

TEST(document_read_preserves_bytes_without_bootstrap,
    source_test_fixture fixture;
    source_test_baseline before;
    TEST_ASSERT(source_test_fixture_open(&fixture, "read", ATTENDANT_FEEDBACK_SOURCE));
    TEST_ASSERT(source_test_baseline_create(&before, fixture.root));
    attendant_result result = attendant_read_document(source_test_session, fixture.target);
    TEST_ASSERT(result.ok && !strcmp(result.text, source_test_saved) && !strcmp(result.hash, fixture.hash));
    TEST_ASSERT(result.file_size == strlen(source_test_saved));
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){before.path}), ((cstring){fixture.root}));
)

TEST(stale_save_preserves_source_and_session_recovers,
    source_test_fixture fixture;
    source_test_baseline before;
    TEST_ASSERT(source_test_fixture_open(&fixture, "stale-save", ATTENDANT_FEEDBACK_SOURCE));
    TEST_ASSERT(source_test_baseline_create(&before, fixture.root));
    attendant_result result = attendant_save_document(source_test_session, fixture.target, "0000000000000000000000000000000000000000000000000000000000000000", "replacement");
    TEST_ASSERT(!result.ok && !strcmp(result.code, "source-changed") && result.written_count == 0);
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){before.path}), ((cstring){fixture.root}));
    TEST_ASSERT(source_test_recovered(&fixture));
)

TEST(atomic_save_preserves_utf8_and_crlf,
    source_test_fixture fixture;
    TEST_ASSERT(source_test_fixture_open(&fixture, "atomic-save", ATTENDANT_FEEDBACK_SOURCE));
    attendant_result result = attendant_save_document(source_test_session, fixture.target, fixture.hash, source_test_original);
    TEST_ASSERT(result.ok && result.written_count == 1 && result.file_size == strlen(source_test_original));
    char hash[65]; snprintf(hash, sizeof(hash), "%s", result.hash);
    TEST_ASSERT_FILE_TEXT(((cstring){fixture.target}), source_test_original);
    result = attendant_read_document(source_test_session, fixture.target);
    TEST_ASSERT(result.ok && !strcmp(result.hash, hash) && !strcmp(result.text, source_test_original));
)

TEST(source_feedback_preflight_leaves_no_files,
    source_test_fixture fixture;
    source_test_baseline before;
    TEST_ASSERT(source_test_fixture_open(&fixture, "source-preflight", ATTENDANT_FEEDBACK_SOURCE));
    TEST_ASSERT(source_test_baseline_create(&before, fixture.root));
    for (int i = 0; i < 6; i++) {
        attendant_feedback input = fixture.input;
        if (i == 0) input.target_hash = "0000000000000000000000000000000000000000000000000000000000000000";
        if (i == 1) input.selected_text = "wrong";
        if (i == 2) input.range_end++;
        if (i == 3) input.range_end = UINT64_MAX;
        if (i == 4) input.range_start = input.range_end = 0;
        if (i == 5) input.comment = " \r\n";
        attendant_result result = attendant_send_feedback(source_test_session, &input);
        TEST_ASSERT(!result.ok && result.written_count == 0);
        TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){before.path}), ((cstring){fixture.root}));
        TEST_ASSERT(source_test_recovered(&fixture));
    }
)

TEST(context_feedback_requires_explicit_absent_anchor,
    source_test_fixture fixture;
    source_test_baseline before;
    TEST_ASSERT(source_test_fixture_open(&fixture, "context-preflight", ATTENDANT_FEEDBACK_CONTEXT));
    TEST_ASSERT(source_test_baseline_create(&before, fixture.root));
    for (int i = 0; i < 5; i++) {
        attendant_feedback input = fixture.input;
        if (i == 0) input.mode = ATTENDANT_FEEDBACK_SOURCE;
        if (i == 1) input.target_hash = "0000000000000000000000000000000000000000000000000000000000000000";
        if (i == 2) input.range_start = 1;
        if (i == 3) input.range_end = 1;
        if (i == 4) input.selected_text = "invented";
        attendant_result result = attendant_send_feedback(source_test_session, &input);
        TEST_ASSERT(!result.ok && result.written_count == 0);
        TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){before.path}), ((cstring){fixture.root}));
        TEST_ASSERT(source_test_recovered(&fixture));
    }
)

/* Same publication checks for source and context feedback, without a subprocess. */
static int source_test_bootstrap_failures(attendant_feedback_mode mode, const char *prefix) {
    const char *faults[] = {"staged", "seeds-published", "comment-published"};
    for (int i = 0; i < 3; i++) {
        char name[80], records[SOURCE_PATH_CAPACITY], config[SOURCE_PATH_CAPACITY];
        source_test_fixture fixture;
        source_test_baseline before;
        snprintf(name, sizeof(name), "%s-%s", prefix, faults[i]);
        TEST_ASSERT(source_test_fixture_open(&fixture, name, mode));
        TEST_ASSERT(source_test_baseline_create(&before, fixture.root));
        TEST_ASSERT(source_test_environment("ATTENDANT_FEEDBACK_FAULT", faults[i]));
        attendant_result result = attendant_send_feedback(source_test_session, &fixture.input);
        TEST_ASSERT(source_test_environment("ATTENDANT_FEEDBACK_FAULT", NULL));
        TEST_ASSERT(!result.ok);
#if defined(_WIN32)
        int published = 0;
#else
        int published = i ? i + 2 : 0;
#endif
        TEST_ASSERT(result.written_count == published);
        for (int j = 0; j < result.written_count; j++) TEST_ASSERT(source_stat(result.written[j]).kind == SOURCE_REGULAR);
        TEST_ASSERT_FILE_TEXT(((cstring){fixture.target}), source_test_saved);
        arena_position_marker mark = arena_position_mark(source_test_files.memory);
        file_tree after;
        TEST_ASSERT_FILE_OK(file_directory_list_tree(&source_test_files, (cstring){fixture.root}, &after));
        for (usize j = 0; j < after.count; j++) {
            const file_tree_entry *entry = &after.entries[j];
            if (entry->kind != file_entry_regular) continue;
            char path[NATIVE_PATH_SIZE], absolute[SOURCE_PATH_CAPACITY];
            TEST_ASSERT_FILE_OK(file_tree_join(path, fixture.root, entry->path));
            if (!source_path_compare(path, fixture.target)) continue;
            TEST_ASSERT(source_full_path(path, absolute));
            int found = 0;
            for (int k = 0; k < result.written_count; k++) if (!source_path_compare(absolute, result.written[k])) found = 1;
            TEST_ASSERT(found); /* No unreported regular files or staged temporaries. */
        }
        TEST_ASSERT(arena_pop_to_marker(mark) == arena_error_ok);
        if (published) TEST_ASSERT(strstr(result.message, "retained published files"));
        snprintf(records, sizeof(records), "%s/records", fixture.root);
        snprintf(config, sizeof(config), "%s/.pi", fixture.root);
        if (published) TEST_ASSERT_FILE_OK(file_directory_remove_tree(&source_test_files, (cstring){records}));
        TEST_ASSERT(source_stat(config).kind == SOURCE_MISSING);
        TEST_ASSERT(source_stat(records).kind == SOURCE_MISSING);
        TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){before.path}), ((cstring){fixture.root}));
        TEST_ASSERT(source_test_recovered(&fixture));
    }
    return 0;
}
TEST(source_feedback_publication_failures,
    TEST_ASSERT(source_test_bootstrap_failures(ATTENDANT_FEEDBACK_SOURCE, "source-failure") == 0);
)
TEST(context_feedback_publication_failures,
    TEST_ASSERT(source_test_bootstrap_failures(ATTENDANT_FEEDBACK_CONTEXT, "context-failure") == 0);
)

static int source_test_target_change(attendant_feedback_mode mode, const char *name) {
    source_test_fixture fixture;
    char expected[1024], path[SOURCE_PATH_CAPACITY];
    TEST_ASSERT(source_test_fixture_open(&fixture, name, mode));
    TEST_ASSERT(source_test_environment("ATTENDANT_FEEDBACK_FAULT", "target-change"));
    attendant_result result = attendant_send_feedback(source_test_session, &fixture.input);
    TEST_ASSERT(source_test_environment("ATTENDANT_FEEDBACK_FAULT", NULL));
    TEST_ASSERT(!result.ok && !strcmp(result.code, "feedback-stale") && !result.written_count);
    snprintf(expected, sizeof(expected), "%s%s", source_test_saved, source_test_external);
    TEST_ASSERT_FILE_TEXT(((cstring){fixture.target}), expected);
    snprintf(path, sizeof(path), "%s/records", fixture.root); TEST_ASSERT(source_stat(path).kind == SOURCE_MISSING);
    snprintf(path, sizeof(path), "%s/.pi", fixture.root); TEST_ASSERT(source_stat(path).kind == SOURCE_MISSING);
    TEST_ASSERT(source_test_recovered(&fixture));
    return 0;
}
TEST(source_feedback_preserves_concurrent_target_edit,
    TEST_ASSERT(source_test_target_change(ATTENDANT_FEEDBACK_SOURCE, "source-target-change") == 0);
)
TEST(context_feedback_preserves_concurrent_target_edit,
    TEST_ASSERT(source_test_target_change(ATTENDANT_FEEDBACK_CONTEXT, "context-target-change") == 0);
)

TEST(feedback_preserves_external_config_publication,
    source_test_fixture fixture;
    char path[SOURCE_PATH_CAPACITY];
    TEST_ASSERT(source_test_fixture_open(&fixture, "config-change", ATTENDANT_FEEDBACK_SOURCE));
    TEST_ASSERT(source_test_environment("ATTENDANT_FEEDBACK_FAULT", "config-change"));
    attendant_result result = attendant_send_feedback(source_test_session, &fixture.input);
    TEST_ASSERT(source_test_environment("ATTENDANT_FEEDBACK_FAULT", NULL));
#if defined(_WIN32)
    TEST_ASSERT(!result.ok && result.written_count == 0);
#else
    TEST_ASSERT(!result.ok && result.written_count == 4);
    for (int i = 0; i < result.written_count; i++) TEST_ASSERT(source_stat(result.written[i]).kind == SOURCE_REGULAR);
#endif
    snprintf(path, sizeof(path), "%s/.pi/attendant.tables", fixture.root);
    TEST_ASSERT_FILE_TEXT(((cstring){path}), source_test_external);
    TEST_ASSERT_FILE_TEXT(((cstring){fixture.target}), source_test_saved);
    snprintf(path, sizeof(path), "%s/records", fixture.root);
#if defined(_WIN32)
    TEST_ASSERT(source_stat(path).kind == SOURCE_MISSING);
#else
    TEST_ASSERT_FILE_OK(file_directory_remove_tree(&source_test_files, (cstring){path}));
#endif
    TEST_ASSERT(source_test_recovered(&fixture));
)

TEST(feedback_preserves_changed_seed_and_rejects_retry,
    source_test_fixture fixture;
    source_test_baseline before;
    char seed[SOURCE_PATH_CAPACITY];
    TEST_ASSERT(source_test_fixture_open(&fixture, "published-edit", ATTENDANT_FEEDBACK_SOURCE));
    TEST_ASSERT(source_test_environment("ATTENDANT_FEEDBACK_FAULT", "published-edit"));
    attendant_result result = attendant_send_feedback(source_test_session, &fixture.input);
    TEST_ASSERT(source_test_environment("ATTENDANT_FEEDBACK_FAULT", NULL));
#if defined(_WIN32)
    TEST_ASSERT(!result.ok && result.written_count == 1 && strstr(result.message, "preserved changed or busy files"));
#else
    TEST_ASSERT(!result.ok && result.written_count == 4 && strstr(result.message, "retained published files"));
#endif
    snprintf(seed, sizeof(seed), "%s/records/_meta_comments/.schema.md", fixture.root);
    char canonical[SOURCE_PATH_CAPACITY]; TEST_ASSERT(source_full_path(seed, canonical));
    int reported = 0;
    for (int i = 0; i < result.written_count; i++) {
        TEST_ASSERT(source_stat(result.written[i]).kind == SOURCE_REGULAR);
        if (!source_path_compare(result.written[i], canonical)) reported = 1;
    }
    TEST_ASSERT(reported);
    TEST_ASSERT_FILE_ENDS_WITH_TEXT(((cstring){seed}), source_test_external);
    TEST_ASSERT(source_test_baseline_create(&before, fixture.root));
    result = attendant_send_feedback(source_test_session, &fixture.input);
    TEST_ASSERT(!result.ok && !strcmp(result.code, "feedback-conflict"));
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){before.path}), ((cstring){fixture.root}));
    TEST_ASSERT(source_test_recovered(&fixture));
)

static int source_test_first_send(attendant_feedback_mode mode, const char *name) {
    source_test_fixture fixture;
    attendant_comment comment;
    char path[SOURCE_PATH_CAPACITY], document[SOURCE_PATH_CAPACITY];
    source_test_baseline before;
    TEST_ASSERT(source_test_fixture_open(&fixture, name, mode));
    attendant_result result = attendant_send_feedback(source_test_session, &fixture.input);
    TEST_ASSERT(result.ok && result.written_count == 5);
    snprintf(document, sizeof(document), "%s", result.path);
    for (int i = 0; i < result.written_count; i++) TEST_ASSERT(source_stat(result.written[i]).kind == SOURCE_REGULAR);
    TEST_ASSERT_FILE_ENDS_WITH_TEXT(((cstring){document}), mode == ATTENDANT_FEEDBACK_SOURCE ? source_test_comment : source_test_context);
    TEST_ASSERT_FILE_TEXT(((cstring){fixture.target}), source_test_saved);
    snprintf(path, sizeof(path), "%s/.attendant", fixture.root); TEST_ASSERT(source_stat(path).kind == SOURCE_MISSING);
    snprintf(path, sizeof(path), "%s/.pi/attendant.tables", fixture.root);
    TEST_ASSERT_FILE_TEXT(((cstring){path}), "records/_meta_comments as _meta_comments\n");
    TEST_ASSERT(source_test_baseline_create(&before, fixture.root));
    result = attendant_read_comment(source_test_session, fixture.root, document, &comment);
    TEST_ASSERT(result.ok && !strcmp(comment.status, "pending") && !strcmp(comment.target_hash, fixture.hash));
    TEST_ASSERT(comment.range_start == fixture.input.range_start && comment.range_end == fixture.input.range_end);
    TEST_ASSERT(!strcmp(comment.selected_text, fixture.input.selected_text) && !strcmp(comment.body, fixture.input.comment));
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){before.path}), ((cstring){fixture.root}));
    return 0;
}
TEST(first_source_feedback_publishes_without_projection,
    TEST_ASSERT(source_test_first_send(ATTENDANT_FEEDBACK_SOURCE, "first-source") == 0);
)
TEST(first_context_feedback_preserves_body_and_absent_anchor,
    TEST_ASSERT(source_test_first_send(ATTENDANT_FEEDBACK_CONTEXT, "first-context") == 0);
)

TEST(later_feedback_creates_only_one_comment,
    source_test_fixture fixture;
    source_test_baseline before;
    TEST_ASSERT(source_test_fixture_open(&fixture, "later-send", ATTENDANT_FEEDBACK_SOURCE));
    TEST_ASSERT(attendant_send_feedback(source_test_session, &fixture.input).ok);
    TEST_ASSERT(source_test_baseline_create(&before, fixture.root));
    attendant_result result = attendant_send_feedback(source_test_session, &fixture.input);
    TEST_ASSERT(result.ok && result.written_count == 1);
    TEST_ASSERT_FILE_OK(file_remove((cstring){(char *)result.path}));
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){before.path}), ((cstring){fixture.root}));
)

TEST(managed_schema_conflict_is_recoverable_and_unchanged,
    source_test_fixture fixture;
    source_test_baseline before;
    char schema[SOURCE_PATH_CAPACITY];
    TEST_ASSERT(source_test_fixture_open(&fixture, "schema-conflict", ATTENDANT_FEEDBACK_SOURCE));
    TEST_ASSERT(attendant_send_feedback(source_test_session, &fixture.input).ok);
    snprintf(schema, sizeof(schema), "%s/records/_meta_comments/.schema.md", fixture.root);
    const char *selected = strstr(comment_schema, "selected_text: \"\"");
    char changed[sizeof(comment_schema) + 8];
    TEST_ASSERT(selected != NULL);
    snprintf(changed, sizeof(changed), "%.*sselected_text: 0%s", (int)(selected - comment_schema), comment_schema, selected + strlen("selected_text: \"\""));
    TEST_ASSERT_FILE_OK(file_remove((cstring){schema}));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){schema}, changed));
    TEST_ASSERT(source_test_baseline_create(&before, fixture.root));
    attendant_result result = attendant_send_feedback(source_test_session, &fixture.input);
    TEST_ASSERT(!result.ok && !strcmp(result.code, "feedback-conflict"));
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){before.path}), ((cstring){fixture.root}));
    TEST_ASSERT(source_test_recovered(&fixture));
)

TEST(alternate_feedback_directories_preserve_config_bytes,
    for (int absolute = 0; absolute < 2; absolute++) {
        source_test_fixture fixture;
        char from[SOURCE_PATH_CAPACITY], to[SOURCE_PATH_CAPACITY], canonical[SOURCE_PATH_CAPACITY], config[SOURCE_PATH_CAPACITY], text[SOURCE_PATH_CAPACITY + 80];
        TEST_ASSERT(source_test_fixture_open(&fixture, absolute ? "alternate-absolute" : "alternate-relative", ATTENDANT_FEEDBACK_SOURCE));
        TEST_ASSERT(attendant_send_feedback(source_test_session, &fixture.input).ok);
        snprintf(from, sizeof(from), "%s/records/_meta_comments", fixture.root);
        snprintf(to, sizeof(to), "%s/feedback elsewhere", fixture.root);
        TEST_ASSERT(source_publish(from, to, SOURCE_CREATE));
        TEST_ASSERT(source_full_path(to, canonical));
        for (char *p = canonical; *p; p++) if (*p == '\\') *p = '/';
        snprintf(config, sizeof(config), "%s/.pi/attendant.tables", fixture.root);
        snprintf(text, sizeof(text), "# Human config comment\r\n%s as _meta_comments\r\n", absolute ? canonical : "feedback elsewhere");
        TEST_ASSERT_FILE_OK(file_remove((cstring){config}));
        TEST_ASSERT_FILE_OK(file_create_text((cstring){config}, text));
        attendant_result result = attendant_send_feedback(source_test_session, &fixture.input);
        TEST_ASSERT(result.ok && result.written_count == 1);
        char expected[SOURCE_PATH_CAPACITY]; TEST_ASSERT(source_full_path(to, expected));
        size_t prefix = strlen(expected);
        TEST_ASSERT(!strncmp(result.path, expected, prefix) && (result.path[prefix] == '/' || result.path[prefix] == '\\'));
        TEST_ASSERT_FILE_TEXT(((cstring){config}), text);
        TEST_ASSERT(source_stat(from).kind == SOURCE_MISSING);
    }
)

TEST(readonly_save_preserves_source_and_removes_staging,
    source_test_fixture fixture;
    source_test_baseline before;
    TEST_ASSERT(source_test_fixture_open(&fixture, "readonly-save", ATTENDANT_FEEDBACK_SOURCE));
    TEST_ASSERT(source_test_baseline_create(&before, fixture.root));
    TEST_ASSERT_FILE_OK(file_set_read_only((cstring){fixture.target}, 1));
    attendant_result result = attendant_save_document(source_test_session, fixture.target, fixture.hash, "must not replace");
    int restored = file_set_read_only((cstring){fixture.target}, 0) == file_error_ok;
    TEST_ASSERT(restored && !result.ok && result.written_count == 0);
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){before.path}), ((cstring){fixture.root}));
    TEST_ASSERT(source_test_recovered(&fixture));
)

TEST(failed_send_never_rolls_back_successful_save,
    source_test_fixture fixture;
    char later[1024], hash[65];
    TEST_ASSERT(source_test_fixture_open(&fixture, "saved-not-queued", ATTENDANT_FEEDBACK_SOURCE));
    TEST_ASSERT(attendant_send_feedback(source_test_session, &fixture.input).ok);
    snprintf(later, sizeof(later), "%sAnother edit.\n", source_test_saved);
    attendant_result result = attendant_save_document(source_test_session, fixture.target, fixture.hash, later);
    TEST_ASSERT(result.ok); snprintf(hash, sizeof(hash), "%s", result.hash); fixture.input.target_hash = hash;
    TEST_ASSERT(source_test_environment("ATTENDANT_FEEDBACK_FAULT", "staged"));
    result = attendant_send_feedback(source_test_session, &fixture.input);
    TEST_ASSERT(source_test_environment("ATTENDANT_FEEDBACK_FAULT", NULL));
    TEST_ASSERT(!result.ok && !result.written_count);
    TEST_ASSERT_FILE_TEXT(((cstring){fixture.target}), later);
    TEST_ASSERT(source_test_recovered(&fixture));
)

TEST(nonempty_unregistered_feedback_directory_is_not_adopted,
    source_test_fixture fixture;
    source_test_baseline before;
    char path[SOURCE_PATH_CAPACITY];
    TEST_ASSERT(source_test_fixture_open(&fixture, "unregistered-conflict", ATTENDANT_FEEDBACK_SOURCE));
    snprintf(path, sizeof(path), "%s/records", fixture.root); TEST_ASSERT(file_directory_create(path, 1));
    snprintf(path, sizeof(path), "%s/records/_meta_comments", fixture.root); TEST_ASSERT(file_directory_create(path, 1));
    snprintf(path, sizeof(path), "%s/records/_meta_comments/keep.md", fixture.root);
    TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "Do not overwrite."));
    TEST_ASSERT(source_test_baseline_create(&before, fixture.root));
    attendant_result result = attendant_send_feedback(source_test_session, &fixture.input);
    TEST_ASSERT(!result.ok && !strcmp(result.code, "feedback-conflict"));
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){before.path}), ((cstring){fixture.root}));
    TEST_ASSERT(source_test_recovered(&fixture));
)
