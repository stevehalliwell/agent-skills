#include "gd.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
#endif

/* TakeScreenshot discards directories in raylib 5.5. Encode explicitly so the
   requested path, including Windows Unicode, is the actual destination. */
int gd_save_screenshot(const char *path) {
    Image image = LoadImageFromScreen();
    int size = 0, ok = 0;
    unsigned char *png = ExportImageToMemory(image, ".png", &size);
    FILE *file = NULL;
    UnloadImage(image);
    if (!png || size <= 0) { MemFree(png); return 0; }
#ifdef _WIN32
    {
        wchar_t wide[GD_MAX_PATH];
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, GD_MAX_PATH))
            file = _wfopen(wide, L"wb");
    }
#else
    file = fopen(path, "wb");
#endif
    if (file) {
        ok = fwrite(png, 1, (size_t)size, file) == (size_t)size && !ferror(file);
        if (fclose(file)) ok = 0;
    }
    MemFree(png);
    return ok;
}

static const Color background = {18, 24, 34, 255};
static const Color panel = {29, 38, 51, 255};
static const Color foreground = {226, 233, 240, 255};
static const Color muted = {160, 179, 199, 255};
static const Color accent = {66, 178, 167, 255};
static const Color selected_color = {66, 109, 162, 255};
static const Color warning = {236, 190, 102, 255};

static int cell_index(const GD_Model *model, int noun, int verb) {
    int low = 0, high = model->cell_count;
    while (low < high) {
        int middle = low + (high-low)/2;
        const GD_Cell *cell = &model->cells[middle];
        if (cell->noun < noun || (cell->noun == noun && cell->verb < verb)) low = middle + 1;
        else high = middle;
    }
    return low < model->cell_count && model->cells[low].noun == noun && model->cells[low].verb == verb ? low : -1;
}
static int button(Rectangle area, const char *label) {
    int hover = CheckCollisionPointRec(GetMousePosition(), area);
    DrawRectangleRec(area, hover ? selected_color : panel);
    DrawText(label, (int)area.x + 8, (int)area.y + 6, 16, foreground);
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
static void clipped_label(const char *text, int x, int y, int width, int size, Color color) {
    BeginScissorMode(x, y, width, size + 5);
    DrawText(text, x, y, size, color);
    EndScissorMode();
}
/* Raw Markdown preview. Wrap UTF-8 codepoints without cutting byte sequences. */
static void body_text(const char *text, int x, float y, int width, int top, int bottom) {
    char line[1024];
    int used = 0;
    const int size = 16, height = 21;
    while (*text && y < bottom) {
        int bytes = 0, codepoint = GetCodepointNext(text, &bytes), w;
        if (bytes <= 0 || bytes > 4) break;
        if (codepoint == '\r') { text += bytes; continue; }
        w = width + 1;
        if (used + bytes < (int)sizeof(line)) {
            memcpy(line + used, text, bytes); line[used + bytes] = 0;
            w = MeasureText(line, size);
        }
        if (codepoint == '\n' || w > width || used + bytes >= (int)sizeof(line)) {
            line[used] = 0;
            if (y + height >= top) DrawText(line, x, (int)y, size, foreground);
            y += height; used = 0;
            if (codepoint == '\n') { text += bytes; continue; }
        }
        memcpy(line + used, text, bytes); used += bytes;
        text += bytes;
    }
    line[used] = 0;
    if (y + height >= top && y < bottom) DrawText(line, x, (int)y, size, foreground);
}
static int fill_axes(const GD_Model *model, int *nouns, int *verbs, int *verb_count) {
    int i, count = 0;
    *verb_count = 0;
    for (i = 0; i < model->record_count; ++i) {
        if (model->records[i].role == GD_NOUN) nouns[count++] = i;
        if (model->records[i].role == GD_VERB) verbs[(*verb_count)++] = i;
    }
    return count;
}
int gd_view(GD_Model *initial, int frames, const char *screenshot) {
    GD_Model *model = initial;
    int nouns[GD_MAX_RECORDS], verbs[GD_MAX_RECORDS], noun_count, verb_count;
    int row = 0, column = 0, first_row = 0, first_column = 0, case_number = 0, linked = -1;
    int frame = 0, stale = 0, result = 0;
    float scroll = 0;
    double next_check = 0;
    char reload_message[256] = "";
    noun_count = fill_axes(model, nouns, verbs, &verb_count);
    if (model->cell_count) {
        int i;
        for (i = 0; i < noun_count; ++i) if (nouns[i] == model->cells[0].noun) row = i;
        for (i = 0; i < verb_count; ++i) if (verbs[i] == model->cells[0].verb) column = i;
    }
    SetTraceLogLevel(LOG_WARNING); /* raylib diagnostics never contaminate command stdout. */
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 800, "Game Design Workbench — interaction matrix");
    if (!IsWindowReady()) { fputs("gd: cannot initialize native matrix window\n", stderr); return 2; }
    SetWindowMinSize(960, 600); SetTargetFPS(60);
    while (!WindowShouldClose() && (!frames || frame < frames)) {
        int width = GetScreenWidth(), height = GetScreenHeight();
        int split = width * 57 / 100, grid_left = 210, grid_top = 128, row_height = 38, column_width = 148;
        int visible_rows = (height-grid_top-52)/row_height, visible_columns = (split-grid_left-16)/column_width;
        int old_row = row, old_column = column, old_case = case_number, cell, i, j, case_record = -1;
        Vector2 mouse = GetMousePosition(); float wheel = GetMouseWheelMove();
        if (GetTime() >= next_check) { if (gd_sources_changed(model)) stale = 1; next_check = GetTime() + 1.0; }
        if (IsKeyPressed(KEY_F5)) {
            GD_Model *fresh = gd_create(model->allocator);
            char noun_reference[260] = "", verb_reference[260] = "";
            if (noun_count) snprintf(noun_reference, sizeof(noun_reference), "%s", model->records[nouns[row]].reference);
            if (verb_count) snprintf(verb_reference, sizeof(verb_reference), "%s", model->records[verbs[column]].reference);
            if (fresh && gd_load(fresh, model->collections, model->collection_count)) {
                GD_Model *old = model; model = fresh;
                noun_count = fill_axes(model, nouns, verbs, &verb_count); row = column = 0;
                for (i = 0; i < noun_count; ++i) if (!strcmp(noun_reference, model->records[nouns[i]].reference)) row = i;
                for (i = 0; i < verb_count; ++i) if (!strcmp(verb_reference, model->records[verbs[i]].reference)) column = i;
                if (old != initial) gd_destroy(old);
                stale = 0; reload_message[0] = 0; case_number = 0; linked = -1; scroll = 0;
            } else {
                stale = 1;
                snprintf(reload_message, sizeof(reload_message), "%s", fresh && fresh->error_count ? fresh->errors[0].message : "Cannot allocate reload storage");
                fprintf(stderr, "gd: refresh failed; retaining stale view: %s\n", reload_message);
                gd_destroy(fresh);
            }
        }
        if (noun_count && verb_count) {
            if (IsKeyPressed(KEY_DOWN) && row + 1 < noun_count) ++row;
            if (IsKeyPressed(KEY_UP) && row > 0) --row;
            if (IsKeyPressed(KEY_RIGHT) && column + 1 < verb_count) ++column;
            if (IsKeyPressed(KEY_LEFT) && column > 0) --column;
        }
        if (wheel && mouse.x < split) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) first_column -= (int)wheel;
            else first_row -= (int)wheel;
        } else if (wheel) scroll -= wheel * 42;
        if (first_row < 0) first_row = 0;
        if (first_column < 0) first_column = 0;
        if (first_row > noun_count-1) first_row = noun_count ? noun_count-1 : 0;
        if (first_column > verb_count-1) first_column = verb_count ? verb_count-1 : 0;
        if (!frame || old_row != row || old_column != column) {
            if (row < first_row) first_row = row;
            if (row >= first_row + visible_rows) first_row = row-visible_rows+1;
            if (column < first_column) first_column = column;
            if (column >= first_column + visible_columns) first_column = column-visible_columns+1;
            case_number = 0; linked = -1; scroll = 0;
        }
        if (scroll < 0) scroll = 0;
        BeginDrawing(); ClearBackground(background);
        DrawText("Interaction matrix", 20, 18, 26, foreground);
        DrawText(TextFormat("%d source records | %d declared cells | %d unplaced cases", model->record_count, model->cell_count, model->unplaced_cases), 20, 54, 16, muted);
        DrawText("Arrows: select | Wheel: pan rows | Shift+wheel: columns | F5: reload", 20, 82, 16, muted);
        if (stale) {
            DrawRectangle(0, height-34, width, 34, (Color){76, 57, 31, 255});
            clipped_label(reload_message[0] ? TextFormat("STALE — refresh failed: %s", reload_message) : "STALE — source changed. F5 reloads; displayed data is the last loaded snapshot.", 16, height-26, width-32, 16, warning);
        } else DrawText("Active = accepted + defined case. Other cases stay visible. Blank is not a defect.", 20, height-26, 16, muted);
        for (j = first_column; j < verb_count && j < first_column + visible_columns; ++j) {
            int x = grid_left + (j-first_column)*column_width;
            clipped_label(model->records[verbs[j]].name, x+6, grid_top-25, column_width-10, 16, foreground);
        }
        for (i = first_row; i < noun_count && i < first_row + visible_rows; ++i) {
            int y = grid_top + (i-first_row)*row_height;
            clipped_label(TextFormat("%s (%d)", model->records[nouns[i]].name, model->connectivity[nouns[i]].partners), 20, y+10, grid_left-30, 16, foreground);
            for (j = first_column; j < verb_count && j < first_column + visible_columns; ++j) {
                Rectangle area = {(float)(grid_left+(j-first_column)*column_width), (float)y, column_width-3.0f, row_height-3.0f};
                int index = cell_index(model, nouns[i], verbs[j]);
                int is_selected = row == i && column == j;
                Color color = index >= 0 && model->cells[index].active ? (Color){36, 85, 85, 255} : panel;
                DrawRectangleRec(area, is_selected ? selected_color : color);
                DrawText(index >= 0 ? TextFormat("%d cases%s", model->cells[index].pair_count, model->cells[index].active ? " *" : "") : "-", (int)area.x+10, y+10, 16, foreground);
                if (CheckCollisionPointRec(mouse, area) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    row = i; column = j; case_number = 0; linked = -1; scroll = 0;
                }
            }
        }
        DrawLine(split, 102, split, height-42, muted);
        cell = noun_count && verb_count ? cell_index(model, nouns[row], verbs[column]) : -1;
        if (cell >= 0) {
            const GD_Cell *selected = &model->cells[cell];
            int x = split+18, detail_width = width-x-18, body_top = 294;
            GD_Record *record;
            if (IsKeyPressed(KEY_PAGE_DOWN)) ++case_number;
            if (IsKeyPressed(KEY_PAGE_UP)) --case_number;
            if (case_number < 0) case_number = selected->pair_count-1;
            case_number %= selected->pair_count;
            if (old_case != case_number) { linked = -1; scroll = 0; }
            case_record = model->pairs[selected->first_pair+case_number].interaction;
            record = &model->records[case_record];
            clipped_label(TextFormat("%s x %s", model->records[selected->noun].name, model->records[selected->verb].name), x, 110, detail_width, 20, foreground);
            clipped_label(TextFormat("Case %d/%d: %s", case_number+1, selected->pair_count, record->name), x, 144, detail_width, 17, foreground);
            clipped_label(TextFormat("status: %s | coverage: %s", record->status ? record->status : "unset", record->coverage ? record->coverage : "unset"), x, 172, detail_width, 16, muted);
            if (button((Rectangle){x, 201, 120, 30}, "Previous") && selected->pair_count > 1) { --case_number; if (case_number < 0) case_number += selected->pair_count; linked = -1; scroll = 0; }
            if (button((Rectangle){x+128, 201, 120, 30}, "Next") && selected->pair_count > 1) { case_number = (case_number+1)%selected->pair_count; linked = -1; scroll = 0; }
            if (linked >= 0 && button((Rectangle){x+256, 201, 135, 30}, "Back to case")) { linked = -1; scroll = 0; }
            if (linked >= 0) record = &model->records[linked];
            clipped_label(record->reference, x, 243, detail_width, 17, accent);
            clipped_label(record->source_path, x, 268, detail_width, 14, muted);
            if (IsKeyPressed(KEY_C)) SetClipboardText(record->source_path);
            BeginScissorMode(x, body_top, detail_width, height-body_top-62);
            {
                float y = body_top-scroll;
                for (i = 0; i < model->edge_count; ++i) {
                    GD_Edge *edge = &model->edges[i];
                    int source = linked >= 0 ? linked : case_record;
                    if (edge->source != source) continue;
                    if (y + 24 >= body_top && y < height-62) {
                        char label[400];
                        snprintf(label, sizeof(label), "%s: %s%s", gd_field_name(edge->field), edge->reference, edge->target < 0 ? " [not uniquely loaded]" : "");
                        DrawText(label, x, (int)y, 15, edge->target >= 0 ? accent : muted);
                        if (edge->target >= 0 && CheckCollisionPointRec(mouse, (Rectangle){x, y, detail_width, 22}) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { linked = edge->target; scroll = 0; break; }
                    }
                    y += 24;
                }
                body_text(record->body, x, y+14, detail_width-6, body_top, height-62);
            }
            EndScissorMode();
            DrawText("PageUp/Down: cases | Wheel: detail | C: copy source path", x, height-52, 14, muted);
        } else {
            DrawText(noun_count && verb_count ? "No declared case in this cell." : "No noun/verb axes in the supplied folders.", split+18, 120, 17, foreground);
            DrawText("No rule, exclusion, or defect inferred.", split+18, 150, 16, muted);
        }
        EndDrawing(); ++frame;
        if (screenshot && frame == frames) {
            if (!gd_save_screenshot(screenshot)) { fputs("gd: screenshot export failed\n", stderr); result = 2; }
        }
    }
    CloseWindow();
    if (model != initial) gd_destroy(model);
    return result;
}
