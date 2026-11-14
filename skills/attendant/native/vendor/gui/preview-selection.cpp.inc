// Preview notes retain rendered sections; source mapping is optional.
PreviewSelection Renderer::preview_selection(const char* input, size_t length) const
{
    PreviewSelection result;
    result.interaction = m_source_selection_interaction;
    if (m_source_widget_selection || !m_selection_fragment || m_selection_fragment != m_fragment_id ||
        m_selection_anchor == m_selection_focus || m_fragment_begin != input || m_fragment_end != input + length)
        return result;
    const size_t begin = ImMin(m_selection_anchor, m_selection_focus), end = ImMax(m_selection_anchor, m_selection_focus);
    if (begin >= end) return result;
    constexpr size_t capacity = 4 * 1024 * 1024;
    auto points = [](const std::string& text, size_t limit) {
        size_t count = 0;
        for (size_t i = 0; i < limit; ++i) if (((unsigned char)text[i] & 0xc0) != 0x80) ++count;
        return count;
    };
    auto quote = [](const std::string& text) {
        const char* digits = "0123456789abcdef";
        std::string out = "\"";
        for (unsigned char c : text) {
            if (c == '"' || c == '\\') { out += '\\'; out += (char)c; }
            else if (c < 0x20) { out += "\\u00"; out += digits[c >> 4]; out += digits[c & 15]; }
            else out += (char)c;
        }
        out += '"'; return out;
    };
    bool mapped = true, haveSource = false;
    size_t previousSourceEnd = 0, sections = 0;
    result.context = "{\"sections\":[";
    for (size_t first = 0; first < m_runs.size();) {
        size_t last = first + 1;
        while (last < m_runs.size() && m_runs[last].block == m_runs[first].block) ++last;
        std::string text;
        size_t highlightBegin = std::string::npos, highlightEnd = 0;
        const TextRun* previous = nullptr;
        auto append = [&](const std::string& piece, bool selected) {
            const size_t at = text.size(); text += piece;
            if (selected && !piece.empty()) {
                if (highlightBegin == std::string::npos) highlightBegin = at;
                highlightEnd = text.size();
            }
        };
        for (size_t i = first; i < last; ++i) {
            const TextRun& run = m_runs[i];
            if (run.begin >= run.end || (run.replacement.empty() && run.end > length)) continue;
            const std::string piece = run.replacement.empty() ? std::string(input + run.begin, input + run.end) : run.replacement;
            if (previous && run.begin > previous->end && !text.empty() && text.back() != ' ' && text.back() != '\n' && !piece.empty() && piece[0] != ' ') {
                bool blank = run.begin <= length && previous->end <= length;
                for (size_t at = previous->end; at < run.begin && blank; ++at)
                    blank = input[at] == ' ' || input[at] == '\t' || input[at] == '\n' || input[at] == '\r';
                if (blank) append(" ", previous->end >= begin && previous->end < end);
            }
            const size_t at = text.size(); text += piece;
            if (run.end > begin && run.begin < end) {
                const size_t from = ImMax(begin, run.begin), to = ImMin(end, run.end);
                const size_t localBegin = run.replacement.empty() ? from - run.begin : 0;
                const size_t localEnd = run.replacement.empty() ? to - run.begin : piece.size();
                if (localBegin < localEnd) {
                    if (highlightBegin == std::string::npos) highlightBegin = at + localBegin;
                    highlightEnd = at + localEnd;
                    if (!run.source_bytes || !run.replacement.empty() || to > length || (haveSource && from < previousSourceEnd)) mapped = false;
                    if (!haveSource) result.begin = from;
                    result.end = to; previousSourceEnd = to; haveSource = true;
                }
            }
            previous = &run;
        }
        if (highlightBegin != std::string::npos && highlightBegin < highlightEnd) {
            if (sections++) { result.context += ','; result.text += '\n'; }
            result.text.append(text, highlightBegin, highlightEnd - highlightBegin);
            result.context += "{\"text\":" + quote(text) + ",\"highlight_start\":" + std::to_string(points(text, highlightBegin)) +
                ",\"highlight_end\":" + std::to_string(points(text, highlightEnd)) + ",\"range_unit\":\"unicode-codepoint\"}";
            if (result.context.size() > capacity || result.text.size() > capacity) throw std::length_error("Preview selection context exceeds 4 MiB");
        }
        first = last;
    }
    result.context += "]}";
    if (result.context.size() > capacity) throw std::length_error("Preview selection context exceeds 4 MiB");
    result.has_source_span = mapped && haveSource && result.begin < result.end && result.end <= length;
    if (!result.has_source_span) result.begin = result.end = 0;
    if (result.text.empty()) result.context.clear();
    return result;
}

// Reuse rendered run provenance for find; never search raw markup as preview text.
int Renderer::find_preview(const char* query, bool matchCase, int action, int& count, int& current)
{
    try {
    constexpr size_t capacity = 4 * 1024 * 1024;
    count = current = 0;
    if (!query || text_find_query_length(query) < 0) return 0;
    for (const char* p = query; *p;) {
        unsigned character; int size = ImTextCharFromUtf8(&character, p, query + strlen(query)); char encoded[5];
        int encodedSize = ImTextCharToUtf8(encoded, character);
        if (size <= 0 || size != encodedSize || memcmp(p, encoded, (size_t)size)) return 0;
        p += size;
    }
    if (!*query) { m_find_query.clear(); m_find_count = m_find_current = 0; return 1; }
    bool layoutChanged = m_find_runs.size() != m_runs.size();
    if (!layoutChanged) for (size_t i = 0; i < m_runs.size(); ++i) {
        const auto& a = m_find_runs[i]; const auto& b = m_runs[i];
        if (a.begin != b.begin || a.end != b.end || a.block != b.block || a.source != b.source_bytes || a.replacement != b.replacement) { layoutChanged = true; break; }
    }
    bool textChanged = false;
    if (layoutChanged) {
        std::string previousText = std::move(m_find_text);
        m_find_text.clear(); m_find_sources.clear(); m_find_runs.clear();
        const size_t length = (size_t)(m_fragment_end - m_fragment_begin);
        const TextRun* previous = nullptr;
        auto append = [&](const char* bytes, size_t size, size_t begin, size_t end, bool direct) {
            if (size > capacity - m_find_text.size()) throw std::length_error("Preview find text exceeds 4 MiB");
            if (!size) return;
            size_t start = m_find_text.size();
            m_find_text.append(bytes, size);
            m_find_sources.push_back({start, start + size, begin, end, direct});
        };
        for (const auto& run : m_runs) {
            m_find_runs.push_back({run.begin, run.end, run.block, run.source_bytes, run.replacement});
            if (run.begin >= run.end || (run.replacement.empty() && run.end > length)) continue;
            const char* bytes = run.replacement.empty() ? m_fragment_begin + run.begin : run.replacement.data();
            const size_t size = run.replacement.empty() ? run.end - run.begin : run.replacement.size();
            if (previous) {
                if (run.block != previous->block) append("\n", 1, previous->end, previous->end, false);
                else if (run.begin > previous->end && !m_find_text.empty() && m_find_text.back() != ' ' && m_find_text.back() != '\n' && size && bytes[0] != ' ') {
                    bool blank = run.begin <= length && previous->end <= length;
                    for (size_t i = previous->end; i < run.begin && blank; ++i) blank = m_fragment_begin[i] == ' ' || m_fragment_begin[i] == '\t' || m_fragment_begin[i] == '\n' || m_fragment_begin[i] == '\r';
                    if (blank) append(" ", 1, previous->end, previous->end, false);
                }
            }
            append(bytes, size, run.begin, run.end, run.replacement.empty()); previous = &run;
        }
        textChanged = previousText != m_find_text;
    }
    bool changed = textChanged || m_find_query != query || m_find_case != matchCase;
    if (changed || action == 2) m_find_current = 0;
    else if (action && m_find_count) m_find_current = (m_find_current + (action < 0 ? -1 : 1) + m_find_count) % m_find_count;
    if (changed || layoutChanged || action) {
        m_find_query = query; m_find_case = matchCase;
        text_find_result hit = text_find_scan(m_find_text.data(), m_find_text.size(), query, matchCase, m_find_current);
        m_find_count = hit.count;
        if (m_find_count && hit.begin < hit.end) {
            auto sourceAt = [&](size_t offset, bool after) {
                auto chunk = std::lower_bound(m_find_sources.begin(), m_find_sources.end(), offset,
                    [](const FindSource& source, size_t position) { return source.text_end <= position; });
                if (chunk == m_find_sources.end()) throw std::out_of_range("Preview find provenance missing");
                return chunk->direct ? chunk->source_begin + offset - chunk->text_begin + (after ? 1 : 0) : after ? chunk->source_end : chunk->source_begin;
            };
            size_t begin = sourceAt(hit.begin, false), end = sourceAt(hit.end - 1, true);
            if (begin < end) {
                select_range(m_fragment_id, begin, end); m_source_widget_selection = false; ++m_source_selection_interaction;
                if (changed || action) for (const auto& run : m_runs) if (run.end > begin && run.begin < end) {
                    ImGui::SetScrollFromPosY(run.min.y - ImGui::GetWindowPos().y, 0.35f); break;
                }
            }
        }
    }
    count = m_find_count; current = count ? m_find_current + 1 : 0; return 1;
    } catch (...) {
        m_find_runs.clear(); m_find_text.clear(); m_find_sources.clear(); m_find_query.clear();
        m_find_count = m_find_current = count = current = 0; return 0;
    }
}

