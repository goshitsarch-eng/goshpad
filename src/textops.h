// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <utility>

namespace textops {

struct Range {
    std::size_t start = 0;
    std::size_t end = 0;
    bool operator==(const Range &other) const
    {
        return start == other.start && end == other.end;
    }
};

enum class ReplaceKind { Replaced, Found };

struct ReplaceResult {
    ReplaceKind kind = ReplaceKind::Found;
    std::string text;
    std::optional<Range> next;
    bool operator==(const ReplaceResult &other) const
    {
        return kind == other.kind && text == other.text && next == other.next;
    }
};

bool selection_matches(const std::string &selected, const std::string &needle, bool matchCase);

std::optional<Range> find_from(const std::string &haystack, const std::string &needle, std::size_t start, bool matchCase);

std::optional<Range> find_next(const std::string &text, std::size_t start, const std::string &needle, bool matchCase, bool wrap);

std::optional<ReplaceResult> replace_and_find_next(const std::string &text,
                                                   const std::optional<Range> &selection,
                                                   const std::string &needle,
                                                   const std::string &replacement,
                                                   bool matchCase,
                                                   bool wrap);

std::pair<std::string, std::size_t> replace_all(const std::string &text, const std::string &needle, const std::string &replacement, bool matchCase);

std::optional<std::size_t> goto_line(const std::string &text, std::size_t lineNumber);

std::pair<std::size_t, std::size_t> line_col_at(const std::string &text, std::size_t offset);

std::pair<std::size_t, std::size_t> caret_line_col(const std::string &text, std::size_t line, std::size_t column);

std::size_t offset_at_line_col(const std::string &text, std::size_t line, std::size_t column);

} // namespace textops
