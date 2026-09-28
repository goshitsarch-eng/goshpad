// SPDX-License-Identifier: GPL-3.0-or-later

#include "document.h"

#include "textops.h"

#include <QFile>
#include <QStringDecoder>

#include <KLocalizedString>

namespace {

constexpr int kMaxUndo = 100;

struct Segment {
    QString content;
    QString ending;
};

QVector<Segment> parseSegments(const QString &canonical)
{
    QVector<Segment> segments;
    QString current;
    const int size = canonical.size();
    for (int index = 0; index < size; ++index) {
        const QChar ch = canonical.at(index);
        if (ch == u'\r') {
            QString ending(ch);
            if (index + 1 < size && canonical.at(index + 1) == u'\n') {
                ending.append(u'\n');
                ++index;
            }
            segments.push_back({current, ending});
            current.clear();
        } else if (ch == u'\n') {
            segments.push_back({current, QString(ch)});
            current.clear();
        } else {
            current.append(ch);
        }
    }
    segments.push_back({current, QString()});
    return segments;
}

QString joinSegments(const QVector<Segment> &segments)
{
    QString out;
    for (const Segment &segment : segments) {
        out += segment.content;
        out += segment.ending;
    }
    return out;
}

std::size_t cursorByteOffset(const QString &display, int cursor)
{
    const QByteArray encoded = display.toUtf8();
    int units = 0;
    int bytes = 0;
    const int target = qBound(0, cursor, display.size());
    while (units < target && bytes < encoded.size()) {
        const auto byte = static_cast<unsigned char>(encoded.at(bytes));
        int length = 1;
        if ((byte & 0xE0) == 0xC0) {
            length = 2;
        } else if ((byte & 0xF0) == 0xE0) {
            length = 3;
        } else if ((byte & 0xF8) == 0xF0) {
            length = 4;
        }
        const int consumedUnits = byte >= 0xF0 ? 2 : 1;
        if (units + consumedUnits > target) {
            break;
        }
        bytes += length;
        units += consumedUnits;
    }
    return std::size_t(bytes);
}

std::pair<std::size_t, std::size_t> caretPosition(const QString &display, int cursor)
{
    const QByteArray encoded = display.toUtf8();
    return textops::line_col_at(std::string(encoded.constData(), std::size_t(encoded.size())), cursorByteOffset(display, cursor));
}

} // namespace

Document::Document(QObject *parent)
    : QObject(parent)
{
}

QString Document::displayText() const
{
    return toDisplay(m_canonical);
}

QString Document::canonicalText() const
{
    return m_canonical;
}

bool Document::isDirty() const
{
    return m_canonical != m_saved;
}

QString Document::path() const
{
    return m_path;
}

int Document::cursorPosition() const
{
    return m_cursor;
}

int Document::selectionStart() const
{
    return m_selStart;
}

int Document::selectionEnd() const
{
    return m_selEnd;
}

int Document::line() const
{
    return int(caretPosition(displayText(), m_cursor).first);
}

int Document::column() const
{
    return int(caretPosition(displayText(), m_cursor).second);
}

bool Document::applying() const
{
    return m_applying;
}

int Document::undoCount() const
{
    return m_undo.size();
}

int Document::redoCount() const
{
    return m_redo.size();
}

QString Document::selectedText() const
{
    const int from = qMin(m_selStart, m_selEnd);
    const int to = qMax(m_selStart, m_selEnd);
    if (from == to) {
        return {};
    }
    return displayText().mid(from, to - from);
}

bool Document::hasSelection() const
{
    return m_selStart != m_selEnd;
}

QString Document::loadPath(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return i18n("Could not open file:\n%1: %2", path, file.errorString());
    }
    const QByteArray bytes = file.readAll();
    QStringDecoder decoder(QStringDecoder::Utf8);
    const QString text = decoder.decode(bytes);
    if (decoder.hasError()) {
        return i18n("Could not open file:\n%1: invalid UTF-8", path);
    }
    const bool wasDirty = isDirty();
    const QString previousPath = m_path;
    m_applying = true;
    m_canonical = text;
    m_saved = text;
    m_path = path;
    m_undo.clear();
    m_redo.clear();
    setCursorState(0, 0, 0);
    m_undoCursor = 0;
    m_undoSelStart = 0;
    m_undoSelEnd = 0;
    m_applying = false;
    Q_EMIT displayTextChanged();
    if (wasDirty) {
        Q_EMIT dirtyChanged();
    }
    if (previousPath != m_path) {
        Q_EMIT pathChanged();
    }
    Q_EMIT cursorChanged();
    return {};
}

QString Document::saveTo(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return i18n("Could not save file:\n%1", file.errorString());
    }
    const QByteArray bytes = m_canonical.toUtf8();
    if (file.write(bytes) != bytes.size()) {
        return i18n("Could not save file:\n%1", file.errorString());
    }
    const bool wasDirty = isDirty();
    const QString previousPath = m_path;
    m_saved = m_canonical;
    m_path = path;
    if (wasDirty) {
        Q_EMIT dirtyChanged();
    }
    if (previousPath != m_path) {
        Q_EMIT pathChanged();
    }
    return {};
}

void Document::reset()
{
    const bool wasDirty = isDirty();
    const bool hadPath = !m_path.isEmpty();
    m_applying = true;
    m_canonical.clear();
    m_saved.clear();
    m_path.clear();
    m_undo.clear();
    m_redo.clear();
    setCursorState(0, 0, 0);
    m_undoCursor = 0;
    m_undoSelStart = 0;
    m_undoSelEnd = 0;
    m_applying = false;
    Q_EMIT displayTextChanged();
    if (wasDirty) {
        Q_EMIT dirtyChanged();
    }
    if (hadPath) {
        Q_EMIT pathChanged();
    }
    Q_EMIT cursorChanged();
}

void Document::markClean()
{
    if (m_saved == m_canonical) {
        return;
    }
    m_saved = m_canonical;
    Q_EMIT dirtyChanged();
}

void Document::applyDisplayText(const QString &display, int cursor, int selStart, int selEnd)
{
    if (m_applying) {
        return;
    }
    if (display == displayText()) {
        setCursorState(cursor, selStart, selEnd);
        m_undoCursor = cursor;
        m_undoSelStart = selStart;
        m_undoSelEnd = selEnd;
        return;
    }
    pushUndo();
    const bool wasDirty = isDirty();
    m_canonical = merge(m_canonical, display);
    setCursorState(cursor, selStart, selEnd);
    m_undoCursor = cursor;
    m_undoSelStart = selStart;
    m_undoSelEnd = selEnd;
    Q_EMIT displayTextChanged();
    if (wasDirty != isDirty()) {
        Q_EMIT dirtyChanged();
    }
}

void Document::noteCursor(int cursor, int selStart, int selEnd)
{
    if (m_applying) {
        return;
    }
    setCursorState(cursor, selStart, selEnd);
}

void Document::commitCursor(int cursor, int selStart, int selEnd)
{
    if (m_applying) {
        return;
    }
    setCursorState(cursor, selStart, selEnd);
    m_undoCursor = cursor;
    m_undoSelStart = selStart;
    m_undoSelEnd = selEnd;
}

bool Document::undo()
{
    if (m_undo.isEmpty()) {
        return false;
    }
    m_redo.append(Snapshot{m_canonical, m_undoCursor, m_undoSelStart, m_undoSelEnd});
    const Snapshot snapshot = m_undo.takeLast();
    restore(snapshot);
    return true;
}

bool Document::redo()
{
    if (m_redo.isEmpty()) {
        return false;
    }
    m_undo.append(Snapshot{m_canonical, m_undoCursor, m_undoSelStart, m_undoSelEnd});
    const Snapshot snapshot = m_redo.takeLast();
    restore(snapshot);
    return true;
}

void Document::selectRange(int from, int to)
{
    setCursorState(to, from, to);
    m_undoCursor = to;
    m_undoSelStart = from;
    m_undoSelEnd = to;
}

void Document::pushUndo()
{
    m_undo.append(Snapshot{m_canonical, m_undoCursor, m_undoSelStart, m_undoSelEnd});
    if (m_undo.size() > kMaxUndo) {
        m_undo.removeFirst();
    }
    m_redo.clear();
}

void Document::restore(const Snapshot &snapshot)
{
    const bool wasDirty = isDirty();
    m_applying = true;
    m_canonical = snapshot.canonical;
    setCursorState(snapshot.cursor, snapshot.selStart, snapshot.selEnd);
    m_undoCursor = snapshot.cursor;
    m_undoSelStart = snapshot.selStart;
    m_undoSelEnd = snapshot.selEnd;
    Q_EMIT displayTextChanged();
    Q_EMIT cursorChanged();
    m_applying = false;
    Q_EMIT applyingChanged();
    if (wasDirty != isDirty()) {
        Q_EMIT dirtyChanged();
    }
}

void Document::setCursorState(int cursor, int selStart, int selEnd)
{
    const QString display = displayText();
    const int max = display.size();
    cursor = qBound(0, cursor, max);
    selStart = qBound(0, selStart, max);
    selEnd = qBound(0, selEnd, max);
    if (m_cursor == cursor && m_selStart == selStart && m_selEnd == selEnd) {
        return;
    }
    m_cursor = cursor;
    m_selStart = selStart;
    m_selEnd = selEnd;
    Q_EMIT cursorChanged();
}

QString Document::toDisplay(const QString &canonical)
{
    QString display;
    display.reserve(canonical.size());
    for (int index = 0; index < canonical.size(); ++index) {
        const QChar ch = canonical.at(index);
        if (ch == u'\r') {
            display.append(u'\n');
            if (index + 1 < canonical.size() && canonical.at(index + 1) == u'\n') {
                ++index;
            }
        } else {
            display.append(ch);
        }
    }
    return display;
}

QString Document::merge(const QString &canonical, const QString &display)
{
    const QVector<Segment> oldSegments = parseSegments(canonical);
    const QStringList lines = display.split(u'\n');
    const int oldCount = oldSegments.size();
    const int newCount = lines.size();

    int prefix = 0;
    while (prefix < oldCount && prefix < newCount && oldSegments.at(prefix).content == lines.at(prefix)) {
        ++prefix;
    }
    int suffix = 0;
    while (suffix < oldCount - prefix && suffix < newCount - prefix
           && oldSegments.at(oldCount - 1 - suffix).content == lines.at(newCount - 1 - suffix)) {
        ++suffix;
    }

    QVector<Segment> merged;
    merged.reserve(newCount);
    for (int index = 0; index < prefix; ++index) {
        merged.push_back(oldSegments.at(index));
    }

    const int oldMiddle = oldCount - suffix - prefix;
    const int newMiddle = newCount - suffix - prefix;
    if (oldMiddle == newMiddle) {
        for (int index = 0; index < newMiddle; ++index) {
            Segment segment = oldSegments.at(prefix + index);
            segment.content = lines.at(prefix + index);
            merged.push_back(segment);
        }
    } else if (oldMiddle == 1 && newMiddle >= 1) {
        for (int index = 0; index < newMiddle; ++index) {
            const QString ending = index == newMiddle - 1 ? oldSegments.at(prefix).ending : QString(u'\n');
            merged.push_back({lines.at(prefix + index), ending});
        }
    } else if (newMiddle == 1 && oldMiddle >= 1) {
        merged.push_back({lines.at(prefix), oldSegments.at(prefix + oldMiddle - 1).ending});
    } else if (newMiddle > 0) {
        const int paired = qMin(oldMiddle, newMiddle);
        for (int index = 0; index < paired; ++index) {
            Segment segment = oldSegments.at(prefix + index);
            segment.content = lines.at(prefix + index);
            merged.push_back(segment);
        }
        for (int index = paired; index < newMiddle; ++index) {
            merged.push_back({lines.at(prefix + index), QString(u'\n')});
        }
    }

    for (int index = 0; index < suffix; ++index) {
        merged.push_back(oldSegments.at(oldCount - suffix + index));
    }
    return joinSegments(merged);
}
