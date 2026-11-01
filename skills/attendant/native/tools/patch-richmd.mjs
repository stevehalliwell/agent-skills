// Deterministic RichMd 1.0 patch: strict legacy source proof and selectable preview context.
// Reconstruct from pinned Git source; refuse unknown edits. No acquisition/build.
import { spawnSync } from 'node:child_process';
import { readFileSync, writeFileSync } from 'node:fs';
import { resolve } from 'node:path';
function replace(source, before, after) {
  if (source.split(before).length !== 2) throw new Error(`RichMd patch anchor missing/nonunique: ${before.slice(0, 70)}`);
  return source.replace(before, after);
}
const previewMethods = readFileSync(new URL('../vendor/gui/preview-selection.cpp.inc', import.meta.url), 'utf8');
const methods = `void Renderer::clear_source_selection()
{
    if (m_selection_fragment && GImGui && GImGui->ActiveId == m_selection_fragment)
        ImGui::ClearActiveID();
    m_selection_fragment = m_selection_text_hash = 0;
    m_selection_anchor = m_selection_focus = 0;
    m_source_widget_selection = false;
}

SourceSelection Renderer::source_selection(const char* input, size_t length) const
{
    SourceSelection result;
    result.interaction = m_source_selection_interaction;
    if (m_source_widget_selection) { result.status = SourceSelectionStatus::Unsupported; return result; }
    if (!length || !m_selection_fragment || m_selection_fragment != m_fragment_id || m_selection_anchor == m_selection_focus)
        return result;
    result.status = SourceSelectionStatus::Unsupported;
    size_t begin = ImMin(m_selection_anchor, m_selection_focus), end = ImMax(m_selection_anchor, m_selection_focus);
    if (m_fragment_begin != input || m_fragment_end != input + length || begin >= end || end > length ||
        (begin < length && ((unsigned char)input[begin] & 0xc0) == 0x80) ||
        (end < length && ((unsigned char)input[end] & 0xc0) == 0x80)) return result;
    // ponytail: only exact contiguous original bytes; broader spans need piecewise provenance.
    size_t cursor = begin;
    for (const TextRun& run : m_runs) {
        if (run.end <= begin || run.begin >= end) continue;
        size_t from = ImMax(begin, run.begin), to = ImMin(end, run.end);
        if (!run.source_bytes || !run.replacement.empty() || from != cursor || to > length) return result;
        cursor = to;
    }
    if (cursor != end) return result;
    std::string visible;
    visible_text(begin, end, visible);
    if (visible.size() != end - begin || visible.compare(0, visible.size(), input + begin, end - begin)) return result;
    result.status = SourceSelectionStatus::Exact; result.begin = begin; result.end = end;
    return result;
}

`;
const patches = {
  'imgui_rich_md/rich_md.h': source => {
    source = replace(source, '#include <cstdint>', '#include <cstdint>\n#include <cstddef>');
    return replace(source, '    void RenderRaw(const std::string& markdownString);', `    void RenderRaw(const std::string& markdownString);

    // Local source-selection patch. Call snapshot while exact RenderRaw input remains alive.
    enum class SourceSelectionStatus { None = 0, Exact = 1, Unsupported = 2 };
    struct SourceSelection { SourceSelectionStatus status = SourceSelectionStatus::None; std::size_t begin = 0, end = 0; std::uint64_t interaction = 0; };
    SourceSelection LastSourceSelection(const std::string& exactInput);
    void ClearSourceSelection();
    struct PreviewSelection { bool has_source_span = false; std::size_t begin = 0, end = 0; std::uint64_t interaction = 0; std::string text, context; };
    void RenderSelectable(const std::string& markdownString);
    PreviewSelection LastPreviewSelection(const std::string& exactInput);
    std::uint64_t LastSelectionInteraction();`);
  },
  'imgui_rich_md/internal/rich_md_renderer.h': source => {
    source = replace(source, '\tint print(const char* str, const char* str_end);', '\tint print(const char* str, const char* str_end);\n\tSourceSelection source_selection(const char* input, size_t length) const;\n\tvoid clear_source_selection();\n\tPreviewSelection preview_selection(const char* input, size_t length) const;\n\tbool selectablePreview = false;\n\tstd::uint64_t selection_interaction() const { return m_source_selection_interaction; }');
    source = replace(source, '\tImGuiID m_selection_fragment = 0;', '\tstd::uint64_t m_source_selection_interaction = 0;\n\tbool m_source_widget_selection = false;\n\tImGuiID m_selection_fragment = 0;');
    return replace(source, '\t\tstd::string replacement;      // what it shows instead of its bytes, when not empty (a soft break, a formula)', '\t\tstd::string replacement;      // what it shows instead of its bytes, when not empty (a soft break, a formula)\n\t\tbool source_bytes = false;    // direct interval in current original-input buffer, not synthetic/foreign');
  },
  'imgui_rich_md/internal/rich_md_renderer.cpp': source => {
    source = '#include <stdexcept>\n' + source;
    source = replace(source, '\tif (str >= str_end)\n        return 0;', '\tif (str >= str_end) {\n        clear_source_selection(); m_fragment_begin = str; m_fragment_end = str_end; m_fragment_id = 0; m_runs.clear(); return 0;\n    }');
    source = replace(source, '\tif (str != nullptr && str >= m_fragment_begin && str < str_end && str_end <= m_fragment_end) {', `\tconst auto address = reinterpret_cast<std::uintptr_t>(str), finish = reinterpret_cast<std::uintptr_t>(str_end);
    if (str && str_end && address >= reinterpret_cast<std::uintptr_t>(m_fragment_begin) && address < finish && finish <= reinterpret_cast<std::uintptr_t>(m_fragment_end)) {
        run.source_bytes = true;`);
    source = replace(source, 'void Renderer::update_selection(ImGuiID fragmentId)\n{', methods + previewMethods + 'void Renderer::update_selection(ImGuiID fragmentId)\n{');
    source = replace(source, '\t\tImGui::KeepAliveID(fragmentId);', `\t\tImGui::KeepAliveID(fragmentId);
        if (selectablePreview && ImGui::IsMouseDragging(0) && g.CurrentWindow->ScrollMax.y > 0) {
            const ImGuiIO& io = g.IO;
            const ImRect clip = g.CurrentWindow->InnerClipRect;
            const float direction = io.MousePos.y < clip.Min.y + 12 ? -1.0f : io.MousePos.y > clip.Max.y - 12 ? 1.0f : 0.0f;
            if (direction) ImGui::SetScrollY(ImClamp(g.CurrentWindow->Scroll.y + direction * ImGui::GetTextLineHeight() * 40 * ImMin(io.DeltaTime, 0.05f), 0.0f, g.CurrentWindow->ScrollMax.y));
        }`);
    for (const signature of ['void Renderer::select_at(ImGuiID fragmentId, size_t offset, int clicks)', 'void Renderer::select_all(ImGuiID fragmentId)'])
      source = replace(source, signature + '\n{', signature + '\n{\n    ++m_source_selection_interaction; m_source_widget_selection = false;');
    source = replace(source, '\tif (mouseFree && ImGui::IsMouseClicked(0)) {', '\tif (mouseFree && ImGui::IsMouseClicked(0)) {\n        ++m_source_selection_interaction; m_source_widget_selection = false;');
    source = replace(source, '\tif (mouseFree && onTextLine && ImGui::IsMouseReleased(1)) {', '\tif (mouseFree && onTextLine && ImGui::IsMouseReleased(1)) {\n        ++m_source_selection_interaction; m_source_widget_selection = false;');
    return replace(source, '\tbool mouseFree = ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && g.ActiveId == 0;', `\t// A child widget owns its own selection. Never export stale surrounding text.
    const bool childClick = ImGui::IsMouseClicked(0) && g.HoveredWindow && !(g.HoveredWindow->Flags & ImGuiWindowFlags_Popup) &&
        ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && !ImGui::IsWindowHovered();
    if (childClick || (g.ActiveId && g.ActiveId != fragmentId && g.ActiveIdWindow && !(g.ActiveIdWindow->Flags & ImGuiWindowFlags_Popup) &&
        (g.ActiveIdWindow == g.CurrentWindow || ImGui::IsWindowChildOf(g.ActiveIdWindow, g.CurrentWindow, true)))) {
        clear_source_selection();
        // Child/custom widgets expose no original-source proof, even when their text is not selectable.
        m_source_widget_selection = true;
        ++m_source_selection_interaction;
    }
\tbool mouseFree = ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && g.ActiveId == 0;`);
  },
  'imgui_rich_md/rich_md.cpp': source => {
    source = replace(source, '        bool can_use_child_windows() const override\n        {', '        bool can_use_child_windows() const override\n        {\n            if (selectablePreview) return false;');
    return replace(source, '    const std::vector<Heading>& LastRenderHeadings()', `    SourceSelection LastSourceSelection(const std::string& exactInput)
    {
        MarkdownRenderer* renderer = _Renderer();
        return renderer ? renderer->source_selection(exactInput.data(), exactInput.size()) : SourceSelection{};
    }
    void ClearSourceSelection()
    {
        MarkdownRenderer* renderer = _Renderer();
        if (renderer) renderer->clear_source_selection();
    }

    void RenderSelectable(const std::string& text)
    {
        MarkdownRenderer* renderer = _Renderer();
        if (!renderer) return;
        struct Restore { bool& flag; bool previous; ~Restore() { flag = previous; } } restore{renderer->selectablePreview, renderer->selectablePreview};
        renderer->selectablePreview = true;
        RenderRaw(text);
    }
    PreviewSelection LastPreviewSelection(const std::string& exactInput)
    {
        MarkdownRenderer* renderer = _Renderer();
        return renderer ? renderer->preview_selection(exactInput.data(), exactInput.size()) : PreviewSelection{};
    }

    std::uint64_t LastSelectionInteraction()
    {
        MarkdownRenderer* renderer = _Renderer();
        return renderer ? renderer->selection_interaction() : 0;
    }

    const std::vector<Heading>& LastRenderHeadings()`);
  },
};
export const richMdPatchedFiles = Object.keys(patches);
export function patchRichMd(verifyOnly) {
  const folder = resolve('build/deps/richmd');
  for (const [path, patch] of Object.entries(patches)) {
    const git = spawnSync('git', ['show', `HEAD:${path}`], { cwd: folder, encoding: 'utf8' });
    if (git.status !== 0) throw new Error(git.stderr || 'Cannot read pinned RichMd source');
    const original = git.stdout.replaceAll('\r\n', '\n').trimEnd(), expected = patch(original);
    const current = readFileSync(resolve(folder, path), 'utf8').replaceAll('\r\n', '\n').trimEnd();
    if (current !== original && current !== expected) throw new Error(`Unexpected local RichMd edits: ${path}`);
    if (verifyOnly && current !== expected) throw new Error(`Missing source-selection patch; run node tools/prepare-gui.mjs: ${path}`);
    if (!verifyOnly) writeFileSync(resolve(folder, path), expected + '\n');
  }
}
