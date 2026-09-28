// SPDX-License-Identifier: GPL-3.0-or-later

#include "textops.h"

#include <QString>

namespace textops {
namespace {

bool is_char_boundary(const std::string &text, std::size_t index)
{
    if (index >= text.size()) {
        return index == text.size();
    }
    const auto byte = static_cast<unsigned char>(text[index]);
    return (byte & 0xC0) != 0x80;
}

bool decode_cp(const std::string &text, std::size_t index, char32_t &cp, std::size_t &length)
{
    if (index >= text.size()) {
        return false;
    }
    const auto *bytes = reinterpret_cast<const unsigned char *>(text.data());
    const unsigned char c0 = bytes[index];
    if (c0 < 0x80) {
        cp = c0;
        length = 1;
        return true;
    }
    if ((c0 & 0xE0) == 0xC0 && index + 1 < text.size() && (bytes[index + 1] & 0xC0) == 0x80 && c0 >= 0xC2) {
        cp = (char32_t(c0 & 0x1F) << 6) | (bytes[index + 1] & 0x3F);
        length = 2;
        return true;
    }
    if ((c0 & 0xF0) == 0xE0 && index + 2 < text.size() && (bytes[index + 1] & 0xC0) == 0x80 && (bytes[index + 2] & 0xC0) == 0x80) {
        cp = (char32_t(c0 & 0x0F) << 12) | (char32_t(bytes[index + 1] & 0x3F) << 6) | (bytes[index + 2] & 0x3F);
        if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) {
            return false;
        }
        length = 3;
        return true;
    }
    if ((c0 & 0xF8) == 0xF0 && index + 3 < text.size() && (bytes[index + 1] & 0xC0) == 0x80 && (bytes[index + 2] & 0xC0) == 0x80
        && (bytes[index + 3] & 0xC0) == 0x80 && c0 <= 0xF4) {
        cp = (char32_t(c0 & 0x07) << 18) | (char32_t(bytes[index + 1] & 0x3F) << 12) | (char32_t(bytes[index + 2] & 0x3F) << 6)
            | (bytes[index + 3] & 0x3F);
        if (cp < 0x10000 || cp > 0x10FFFF) {
            return false;
        }
        length = 4;
        return true;
    }
    return false;
}

QString lowercase_cp(char32_t cp)
{
    return QString::fromUcs4(&cp, 1).toLower();
}

std::optional<std::size_t> match_prefix_ci(const std::string &hay, const std::string &needle)
{
    std::size_t hayIndex = 0;
    std::size_t needleIndex = 0;
    while (needleIndex < needle.size()) {
        char32_t needleCp = 0;
        std::size_t needleLen = 0;
        if (!decode_cp(needle, needleIndex, needleCp, needleLen)) {
            return std::nullopt;
        }
        if (hayIndex >= hay.size()) {
            return std::nullopt;
        }
        char32_t hayCp = 0;
        std::size_t hayLen = 0;
        if (!decode_cp(hay, hayIndex, hayCp, hayLen)) {
            return std::nullopt;
        }
        if (lowercase_cp(hayCp) != lowercase_cp(needleCp)) {
            return std::nullopt;
        }
        hayIndex += hayLen;
        needleIndex += needleLen;
    }
    return hayIndex;
}

std::size_t next_boundary(const std::string &text, std::size_t start)
{
    for (std::size_t index = start + 1; index < text.size(); ++index) {
        if (is_char_boundary(text, index)) {
            return index;
        }
    }
    return text.size();
}

std::string lowercase_utf8(const std::string &text)
{
    std::size_t index = 0;
    QString lowered;
    while (index < text.size()) {
        char32_t cp = 0;
        std::size_t length = 0;
        if (!decode_cp(text, index, cp, length)) {
            return QString::fromUtf8(text).toLower().toStdString();
        }
        lowered += lowercase_cp(cp);
        index += length;
    }
    return lowered.toStdString();
}

} // namespace

bool selection_matches(const std::string &selected, const std::string &needle, bool matchCase)
{
    if (needle.empty() || selected.empty()) {
        return false;
    }
    if (matchCase) {
        return selected == needle;
    }
    return lowercase_utf8(selected) == lowercase_utf8(needle);
}

std::optional<Range> find_from(const std::string &haystack, const std::string &needle, std::size_t start, bool matchCase)
{
    if (needle.empty() || start > haystack.size()) {
        return std::nullopt;
    }
    if (!is_char_boundary(haystack, start)) {
        start = next_boundary(haystack, start);
    }
    const std::string hay = haystack.substr(start);
    if (matchCase) {
        const auto found = hay.find(needle);
        if (found == std::string::npos) {
            return std::nullopt;
        }
        const auto from = start + found;
        return Range{from, from + needle.size()};
    }
    std::size_t index = 0;
    while (index < hay.size()) {
        if (!is_char_boundary(hay, index)) {
            ++index;
            continue;
        }
        if (const auto matched = match_prefix_ci(hay.substr(index), needle)) {
            const auto from = start + index;
            return Range{from, from + *matched};
        }
        char32_t cp = 0;
        std::size_t length = 0;
        if (!decode_cp(hay, index, cp, length)) {
            ++index;
        } else {
            index += length;
        }
    }
    return std::nullopt;
}

std::optional<Range> find_next(const std::string &text, std::size_t start, const std::string &needle, bool matchCase, bool wrap)
{
    if (needle.empty()) {
        return std::nullopt;
    }
    if (const auto found = find_from(text, needle, start, matchCase)) {
        return found;
    }
    if (wrap && start > 0) {
        return find_from(text, needle, 0, matchCase);
    }
    return std::nullopt;
}

std::optional<ReplaceResult> replace_and_find_next(const std::string &text,
                                                   const std::optional<Range> &selection,
                                                   const std::string &needle,
                                                   const std::string &replacement,
                                                   bool matchCase,
                                                   bool wrap)
{
    if (needle.empty()) {
        return std::nullopt;
    }
    if (selection) {
        const auto start = selection->start;
        const auto end = selection->end;
        if (end <= text.size() && start <= end && is_char_boundary(text, start) && is_char_boundary(text, end)) {
            const auto selected = text.substr(start, end - start);
            if (selection_matches(selected, needle, matchCase)) {
                std::string nextText;
                nextText.reserve(text.size() - (end - start) + replacement.size());
                nextText.append(text, 0, start);
                nextText.append(replacement);
                nextText.append(text, end, std::string::npos);
                const auto continueAt = start + replacement.size();
                ReplaceResult result;
                result.kind = ReplaceKind::Replaced;
                result.text = std::move(nextText);
                result.next = find_next(result.text, continueAt, needle, matchCase, wrap);
                return result;
            }
        }
        const auto from = selection->end;
        if (const auto found = find_next(text, from, needle, matchCase, wrap)) {
            return ReplaceResult{ReplaceKind::Found, {}, found};
        }
        return std::nullopt;
    }
    if (const auto found = find_next(text, 0, needle, matchCase, wrap)) {
        return ReplaceResult{ReplaceKind::Found, {}, found};
    }
    return std::nullopt;
}

std::pair<std::string, std::size_t> replace_all(const std::string &text, const std::string &needle, const std::string &replacement, bool matchCase)
{
    if (needle.empty()) {
        return {text, 0};
    }
    std::string out;
    out.reserve(text.size());
    std::size_t cursor = 0;
    std::size_t count = 0;
    while (const auto found = find_from(text, needle, cursor, matchCase)) {
        out.append(text, cursor, found->start - cursor);
        out.append(replacement);
        cursor = found->end;
        ++count;
        if (found->start == found->end) {
            break;
        }
    }
    out.append(text, cursor, std::string::npos);
    return {out, count};
}

std::optional<std::size_t> goto_line(const std::string &text, std::size_t lineNumber)
{
    if (lineNumber < 1) {
        return std::nullopt;
    }
    std::size_t line = 1;
    std::size_t offset = 0;
    if (lineNumber == 1) {
        return 0;
    }
    for (std::size_t index = 0; index < text.size(); ++index) {
        if (text[index] == '\n') {
            ++line;
            offset = index + 1;
            if (line == lineNumber) {
                return offset;
            }
        }
    }
    return std::nullopt;
}

std::pair<std::size_t, std::size_t> line_col_at(const std::string &text, std::size_t offset)
{
    if (offset > text.size()) {
        offset = text.size();
    }
    while (offset > 0 && !is_char_boundary(text, offset)) {
        --offset;
    }
    std::size_t line = 1;
    std::size_t lineStart = 0;
    for (std::size_t index = 0; index < offset; ++index) {
        if (text[index] == '\n') {
            ++line;
            lineStart = index + 1;
        }
    }
    std::size_t column = 1;
    std::size_t index = lineStart;
    while (index < offset) {
        char32_t cp = 0;
        std::size_t length = 0;
        if (!decode_cp(text, index, cp, length)) {
            ++index;
            ++column;
            continue;
        }
        index += length;
        ++column;
    }
    return {line, column};
}

std::pair<std::size_t, std::size_t> caret_line_col(const std::string &text, std::size_t line, std::size_t column)
{
    std::size_t currentLine = 0;
    std::size_t currentCol = 0;
    std::size_t displayLine = 1;
    std::size_t displayCol = 1;
    std::size_t index = 0;
    while (index < text.size()) {
        if (currentLine == line && currentCol == column) {
            return {displayLine, displayCol};
        }
        char32_t cp = 0;
        std::size_t length = 0;
        if (!decode_cp(text, index, cp, length)) {
            cp = static_cast<unsigned char>(text[index]);
            length = 1;
        }
        if (cp == U'\n') {
            if (currentLine == line) {
                return {displayLine, displayCol};
            }
            ++currentLine;
            currentCol = 0;
            ++displayLine;
            displayCol = 1;
        } else {
            ++currentCol;
            ++displayCol;
        }
        index += length;
    }
    return {displayLine, displayCol};
}

std::size_t offset_at_line_col(const std::string &text, std::size_t line, std::size_t column)
{
    std::size_t currentLine = 0;
    std::size_t currentCol = 0;
    std::size_t index = 0;
    while (index < text.size()) {
        if (currentLine == line && currentCol == column) {
            return index;
        }
        char32_t cp = 0;
        std::size_t length = 0;
        if (!decode_cp(text, index, cp, length)) {
            cp = static_cast<unsigned char>(text[index]);
            length = 1;
        }
        if (cp == U'\n') {
            if (currentLine == line) {
                return index;
            }
            ++currentLine;
            currentCol = 0;
        } else {
            ++currentCol;
        }
        index += length;
    }
    return text.size();
}

} // namespace textops
