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

