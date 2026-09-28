// SPDX-License-Identifier: GPL-3.0-or-later

#include "controller.h"

#include "settings.h"
#include "textops.h"

#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMimeData>
#include <QStyleHints>

#include <KLocalizedString>

namespace {

constexpr int kFontSizes[] = {8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 28, 32, 36};

QStringList familyList()
{
    return {QStringLiteral("monospace"),
            QStringLiteral("sans-serif"),
            QStringLiteral("serif"),
            QStringLiteral("Noto Sans Mono"),
            QStringLiteral("Open Sans"),
            QStringLiteral("DejaVu Sans Mono"),
            QStringLiteral("Liberation Mono"),
            QStringLiteral("FreeMono"),
            QStringLiteral("Ubuntu Mono"),
            QStringLiteral("Source Code Pro"),
            QStringLiteral("Fira Code"),
            QStringLiteral("JetBrains Mono"),
            QStringLiteral("Noto Sans"),
            QStringLiteral("Noto Serif")};
}

QStringList sizeList()
{
    QStringList sizes;
    for (int size : kFontSizes) {
        sizes.append(QString::number(size));
    }
    return sizes;
}

int sizeIndex(int size)
{
    const QStringList sizes = sizeList();
    const int index = sizes.indexOf(QString::number(size));
    return index < 0 ? sizes.indexOf(QStringLiteral("14")) : index;
}

std::size_t qstringToUtf8(const QString &text, int index)
{
    const QByteArray encoded = text.toUtf8();
    int units = 0;
    int bytes = 0;
    const int target = qBound(0, index, text.size());
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
        const int consumed = byte >= 0xF0 ? 2 : 1;
        if (units + consumed > target) {
            break;
        }
        bytes += length;
        units += consumed;
    }
    return std::size_t(bytes);
}

int utf8ToQString(const QString &text, std::size_t byteOffset)
{
    const QByteArray encoded = text.toUtf8();
    int units = 0;
    int bytes = 0;
    const int limit = int(qMin(byteOffset, std::size_t(encoded.size())));
    while (bytes < limit && bytes < encoded.size()) {
        const auto byte = static_cast<unsigned char>(encoded.at(bytes));
        int length = 1;
        int consumed = 1;
        if ((byte & 0xE0) == 0xC0) {
            length = 2;
        } else if ((byte & 0xF0) == 0xE0) {
            length = 3;
        } else if ((byte & 0xF8) == 0xF0) {
            length = 4;
            consumed = 2;
        }
        if (bytes + length > limit) {
            break;
        }
        bytes += length;
        units += consumed;
    }
    return qMin(units, text.size());
}

QString absolutePath(const QString &path, const QString &workingDirectory)
{
    const QFileInfo info(path);
    if (info.isAbsolute()) {
        return info.absoluteFilePath();
    }
    const QString base = workingDirectory.isEmpty() ? QDir::currentPath() : workingDirectory;
    return QFileInfo(QDir(base).filePath(path)).absoluteFilePath();
}

} // namespace

Controller::Controller(QObject *parent)
    : QObject(parent)
{
    loadSettings();
    applyScheme();
    connect(&m_document, &Document::dirtyChanged, this, &Controller::titleChanged);
    connect(&m_document, &Document::pathChanged, this, &Controller::titleChanged);
    connect(&m_document, &Document::displayTextChanged, this, &Controller::titleChanged);
    connect(&m_document, &Document::cursorChanged, this, &Controller::cursorChanged);
    if (auto *hints = QGuiApplication::styleHints()) {
        connect(hints, &QStyleHints::colorSchemeChanged, this, &Controller::appearanceChanged);
    }
}

Document *Controller::document()
{
    return &m_document;
}

QString Controller::title() const
{
    const QString name = m_document.path().isEmpty() ? i18n("Untitled") : QFileInfo(m_document.path()).fileName();
    const QString marker = m_document.isDirty() ? QStringLiteral("•  ") : QString();
    return marker + name + QStringLiteral(" — ") + i18n("GoshPad");
}

QString Controller::dialog() const
{
    return m_dialog;
}

QString Controller::errorMessage() const
{
    return m_errorMessage;
}

bool Controller::modalOpen() const
{
    return !m_dialog.isEmpty() || m_fileDialogOpen;
}

bool Controller::fileDialogOpen() const
{
    return m_fileDialogOpen;
}

void Controller::setFileDialogOpen(bool open)
{
    if (m_fileDialogOpen == open) {
        return;
    }
    m_fileDialogOpen = open;
    emit dialogChanged();
}

bool Controller::findVisible() const
{
    return m_findVisible;
}

bool Controller::replaceVisible() const
{
    return m_replaceVisible;
}

QString Controller::findText() const
{
    return m_findText;
}

void Controller::setFindText(const QString &text)
{
    if (m_findText == text) {
        return;
    }
    m_findText = text;
    emit findChanged();
}

QString Controller::replaceText() const
{
    return m_replaceText;
}

void Controller::setReplaceText(const QString &text)
{
    if (m_replaceText == text) {
        return;
    }
    m_replaceText = text;
    emit findChanged();
}

bool Controller::matchCase() const
{
    return m_matchCase;
}

void Controller::setMatchCase(bool match)
{
    if (m_matchCase == match) {
        return;
    }
    m_matchCase = match;
    emit findChanged();
}

bool Controller::wordWrap() const
{
    return m_wordWrap;
}

void Controller::setWordWrap(bool enabled)
{
    if (m_wordWrap == enabled) {
        return;
    }
    m_wordWrap = enabled;
    saveSettings();
    emit appearanceChanged();
}

bool Controller::showStatusBar() const
{
    return m_showStatusBar;
}

void Controller::setShowStatusBar(bool enabled)
{
    if (m_showStatusBar == enabled) {
        return;
    }
    m_showStatusBar = enabled;
    saveSettings();
    emit appearanceChanged();
}

QString Controller::fontFamily() const
{
    return m_fontFamily;
}

int Controller::fontSize() const
{
    return m_fontSize;
}

QString Controller::fontFamilyInput() const
{
    return m_fontFamilyInput;
}

void Controller::setFontFamilyInput(const QString &family)
{
    if (m_fontFamilyInput == family) {
        return;
    }
    m_fontFamilyInput = family;
    emit fontDialogChanged();
}

int Controller::fontSizeIndex() const
{
    return m_fontSizeIndex;
}

void Controller::setFontSizeIndex(int index)
{
    const int bounded = qBound(0, index, sizeList().size() - 1);
    if (m_fontSizeIndex == bounded) {
        return;
    }
    m_fontSizeIndex = bounded;
    emit fontDialogChanged();
}

QStringList Controller::fontFamilies() const
{
    return familyList();
}

QStringList Controller::fontSizes() const
{
    return sizeList();
}

QString Controller::colorScheme() const
{
    return m_scheme;
}

bool Controller::effectiveDark() const
{
    if (m_scheme == QLatin1String("dark")) {
        return true;
    }
    if (m_scheme == QLatin1String("light")) {
        return false;
    }
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

QString Controller::schemeButtonText() const
{
    return effectiveDark() ? i18n("Light") : i18n("Dark");
}

QString Controller::schemeButtonTooltip() const
{
    return effectiveDark() ? i18n("Switch to light mode") : i18n("Toggle dark mode");
}

QString Controller::statusText() const
{
    return i18n("Ln %1, Col %2", m_document.line(), m_document.column());
}

QString Controller::wrapLabel() const
{
    return m_wordWrap ? i18n("Word Wrap: On") : i18n("Word Wrap: Off");
}

bool Controller::gotoEnabled() const
{
    return !m_wordWrap;
}

QString Controller::gotoInput() const
{
    return m_gotoInput;
}

void Controller::setGotoInput(const QString &text)
{
    m_gotoInput = text;
    m_gotoError.clear();
    emit gotoChanged();
}

QString Controller::gotoError() const
{
    return m_gotoError;
}

QUrl Controller::currentFolder() const
{
    if (m_document.path().isEmpty()) {
        return QUrl::fromLocalFile(QDir::homePath());
    }
    return QUrl::fromLocalFile(QFileInfo(m_document.path()).absolutePath());
}

QUrl Controller::suggestedSaveUrl() const
{
    const QString name = m_document.path().isEmpty() ? i18n("Untitled") + QStringLiteral(".txt") : QFileInfo(m_document.path()).fileName();
    return QUrl::fromLocalFile(QDir(currentFolder().toLocalFile()).filePath(name));
}

QVariantMap Controller::aboutData() const
{
    QVariantMap license;
    license.insert(QStringLiteral("name"), QStringLiteral("GPL-3.0-or-later"));
    license.insert(QStringLiteral("spdx"), QStringLiteral("GPL-3.0-or-later"));
    license.insert(QStringLiteral("text"), i18n("GNU General Public License, version 3 or later."));
    QVariantMap data;
    data.insert(QStringLiteral("displayName"), i18n("GoshPad"));
    data.insert(QStringLiteral("productName"), QStringLiteral("goshpad"));
    data.insert(QStringLiteral("componentName"), QStringLiteral("goshpad"));
    data.insert(QStringLiteral("shortDescription"), i18n("A plain-text editor in the style of classic Microsoft Notepad"));
    data.insert(QStringLiteral("homepage"), QStringLiteral("https://github.com/goshitsarch-eng/goshpad"));
    data.insert(QStringLiteral("bugAddress"), QStringLiteral("https://github.com/goshitsarch-eng/goshpad/issues"));
    data.insert(QStringLiteral("version"), QStringLiteral(GOSHPAD_VERSION));
    data.insert(QStringLiteral("copyrightStatement"), i18n("© 2026 Gosh and GoshPad contributors"));
    data.insert(QStringLiteral("desktopFileName"), QStringLiteral("com.goshapps.GoshPad"));
    data.insert(QStringLiteral("licenses"), QVariantList{license});
    data.insert(QStringLiteral("authors"), QVariantList{});
    data.insert(QStringLiteral("credits"), QVariantList{});
    data.insert(QStringLiteral("translators"), QVariantList{});
    return data;
}

void Controller::startup(const QString &path)
{
    if (!path.isEmpty()) {
        loadPath(path);
    }
}

void Controller::activate(const QStringList &arguments, const QString &workingDirectory)
{
    emit raiseRequested();
    const QString path = firstPath(arguments, workingDirectory, true);
    if (path.isEmpty()) {
        return;
    }
    m_pendingPath = path;
    guard(After::OpenPath);
}

void Controller::activateUrls(const QList<QUrl> &urls)
{
    emit raiseRequested();
    for (const QUrl &url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }
        m_pendingPath = url.toLocalFile();
        guard(After::OpenPath);
        return;
    }
}

void Controller::newDocument()
{
    if (modalOpen()) {
        return;
    }
    guard(After::NewDoc);
}

void Controller::open()
{
    if (modalOpen()) {
        return;
    }
    guard(After::Open);
}

void Controller::save()
{
    if (modalOpen()) {
        return;
    }
    if (m_document.path().isEmpty()) {
        emit saveDialogRequested();
        return;
    }
    writeTo(m_document.path());
}

void Controller::saveAs()
{
    if (modalOpen()) {
        return;
    }
    emit saveDialogRequested();
}

void Controller::requestClose()
{
    if (m_dialog == QLatin1String("save")) {
        m_after = After::Close;
        return;
    }
    guard(After::Close);
}

void Controller::loadChosen(const QUrl &url)
{
    setFileDialogOpen(false);
    if (!url.isLocalFile()) {
        showError(i18n("Could not open file:\n%1", url.toString()));
        return;
    }
    m_pendingPath = url.toLocalFile();
    guard(After::OpenPath);
}

void Controller::saveChosen(const QUrl &url)
{
    setFileDialogOpen(false);
    if (!url.isLocalFile()) {
        showError(i18n("Could not save file:\n%1", url.toString()));
        return;
    }
    writeTo(url.toLocalFile());
}

void Controller::fileDialogCancelled()
{
    setFileDialogOpen(false);
    m_pendingAfter = After::None;
}

void Controller::undo()
{
    if (modalOpen()) {
        return;
    }
    m_document.undo();
}

void Controller::redo()
{
    if (modalOpen()) {
        return;
    }
    m_document.redo();
}

void Controller::cut()
{
    const QString selected = m_document.selectedText();
    if (selected.isEmpty()) {
        return;
    }
    QGuiApplication::clipboard()->setText(selected);
    insertText(QString());
}

void Controller::copy()
{
    const QString selected = m_document.selectedText();
    if (!selected.isEmpty()) {
        QGuiApplication::clipboard()->setText(selected);
    }
}

void Controller::paste()
{
    const QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard->mimeData() || !clipboard->mimeData()->hasText()) {
        return;
    }
    insertText(clipboard->text());
}

void Controller::deleteText()
{
    if (m_document.hasSelection()) {
        insertText(QString());
        return;
    }
    const QString display = m_document.displayText();
    const int pos = m_document.cursorPosition();
    if (pos >= display.size()) {
        return;
    }
    int next = pos + 1;
    if (display.at(pos).isHighSurrogate() && next < display.size() && display.at(next).isLowSurrogate()) {
        ++next;
    }
    const QString updated = display.left(pos) + display.mid(next);
    m_document.applyDisplayText(updated, pos, pos, pos);
}

void Controller::selectAll()
{
    const int end = m_document.displayText().size();
    m_document.selectRange(0, end);
}

void Controller::showFind()
{
    if (modalOpen()) {
        return;
    }
    prefillFind();
    m_findVisible = true;
    m_replaceVisible = false;
    emit findChanged();
}

void Controller::showReplace()
{
    if (modalOpen()) {
        return;
    }
    prefillFind();
    m_findVisible = true;
    m_replaceVisible = true;
    emit findChanged();
}

void Controller::findNext()
{
    if (m_findText.isEmpty()) {
        return;
    }
    const QString display = m_document.displayText();
    const std::string text = display.toUtf8().toStdString();
    const std::string needle = m_findText.toUtf8().toStdString();
    const int origin = m_document.hasSelection() ? qMax(m_document.selectionStart(), m_document.selectionEnd()) : m_document.cursorPosition();
    const auto found = textops::find_next(text, qstringToUtf8(display, origin), needle, m_matchCase, true);
    if (!found) {
        showError(i18n("Cannot find \"%1\"", m_findText));
        return;
    }
    m_document.selectRange(utf8ToQString(display, found->start), utf8ToQString(display, found->end));
}

void Controller::replaceOne()
{
    if (m_findText.isEmpty()) {
        return;
    }
    const QString display = m_document.displayText();
    const std::string text = display.toUtf8().toStdString();
    const std::string needle = m_findText.toUtf8().toStdString();
    const std::string replacement = m_replaceText.toUtf8().toStdString();
    std::optional<textops::Range> selection;
    if (m_document.hasSelection()) {
        const int from = qMin(m_document.selectionStart(), m_document.selectionEnd());
        const int to = qMax(m_document.selectionStart(), m_document.selectionEnd());
        selection = textops::Range{qstringToUtf8(display, from), qstringToUtf8(display, to)};
    }
    const auto result = textops::replace_and_find_next(text, selection, needle, replacement, m_matchCase, true);
    if (!result) {
        showError(i18n("Cannot find \"%1\"", m_findText));
        return;
    }
    if (result->kind == textops::ReplaceKind::Found) {
        m_document.selectRange(utf8ToQString(display, result->next->start), utf8ToQString(display, result->next->end));
        return;
    }
    const QString updated = QString::fromUtf8(result->text);
    int cursor = updated.size();
    int from = cursor;
    int to = cursor;
    if (result->next) {
        from = utf8ToQString(updated, result->next->start);
        to = utf8ToQString(updated, result->next->end);
        cursor = to;
    }
    m_document.applyDisplayText(updated, cursor, from, to);
}

void Controller::replaceAll()
{
    if (m_findText.isEmpty()) {
        return;
    }
    const QString display = m_document.displayText();
    const auto replaced = textops::replace_all(display.toUtf8().toStdString(), m_findText.toUtf8().toStdString(), m_replaceText.toUtf8().toStdString(), m_matchCase);
    if (replaced.second == 0) {
        showError(i18n("Cannot find \"%1\"", m_findText));
        return;
    }
    const QString updated = QString::fromUtf8(replaced.first);
    const int cursor = qMin(m_document.cursorPosition(), updated.size());
    m_document.applyDisplayText(updated, cursor, cursor, cursor);
}

void Controller::closeFind()
{
    if (!m_findVisible && !m_replaceVisible) {
        return;
    }
    m_findVisible = false;
    m_replaceVisible = false;
    emit findChanged();
}

void Controller::goTo()
{
    if (modalOpen() || m_wordWrap) {
        return;
    }
    m_gotoInput = QString::number(m_document.line());
    m_gotoError.clear();
    emit gotoChanged();
    setDialog(QStringLiteral("goto"));
}

void Controller::confirmGoTo()
{
    bool ok = false;
    const int line = m_gotoInput.trimmed().toInt(&ok);
    if (!ok || line < 1) {
        m_gotoError = i18n("Please enter a valid line number.");
        emit gotoChanged();
        return;
    }
    const QString display = m_document.displayText();
    const auto offset = textops::goto_line(display.toUtf8().toStdString(), std::size_t(line));
    if (!offset) {
        m_gotoError = i18n("The line number is beyond the total number of lines.");
        emit gotoChanged();
        return;
    }
    const int index = utf8ToQString(display, *offset);
    setDialog(QString());
    m_document.selectRange(index, index);
}

void Controller::insertDateTime()
{
    if (modalOpen()) {
        return;
    }
    const QDateTime now = QDateTime::currentDateTime();
    int hour = now.time().hour() % 12;
    if (hour == 0) {
        hour = 12;
    }
    const QString suffix = now.time().hour() < 12 ? QStringLiteral("AM") : QStringLiteral("PM");
    const QString stamp = QStringLiteral("%1:%2 %3 %4/%5/%6")
                              .arg(hour)
                              .arg(now.time().minute(), 2, 10, QLatin1Char('0'))
                              .arg(suffix)
                              .arg(now.date().month())
                              .arg(now.date().day())
                              .arg(now.date().year());
    insertText(stamp);
}

void Controller::showFontDialog()
{
    if (modalOpen()) {
        return;
    }
    m_fontFamilyInput = m_fontFamily;
    m_fontSizeIndex = sizeIndex(m_fontSize);
    emit fontDialogChanged();
    setDialog(QStringLiteral("font"));
}

void Controller::applyFont()
{
    const QString family = m_fontFamilyInput.trimmed().isEmpty() ? QStringLiteral("monospace") : m_fontFamilyInput.trimmed();
    m_fontFamily = family;
    m_fontSize = sizeList().at(m_fontSizeIndex).toInt();
    saveSettings();
    setDialog(QString());
    emit appearanceChanged();
}

void Controller::setColorScheme(const QString &scheme)
{
    const QString next = (scheme == QLatin1String("light") || scheme == QLatin1String("dark")) ? scheme : QStringLiteral("system");
    if (m_scheme == next) {
        applyScheme();
        emit appearanceChanged();
        return;
    }
    m_scheme = next;
    applyScheme();
    saveSettings();
    emit appearanceChanged();
}

void Controller::toggleScheme()
{
    setColorScheme(effectiveDark() ? QStringLiteral("light") : QStringLiteral("dark"));
}

void Controller::showAbout()
{
    if (modalOpen()) {
        return;
    }
    setDialog(QStringLiteral("about"));
}

void Controller::dialogSave()
{
    m_pendingAfter = m_after;
    m_after = After::None;
    setDialog(QString());
    if (m_document.path().isEmpty()) {
        emit saveDialogRequested();
        return;
    }
    writeTo(m_document.path());
}

void Controller::dialogDiscard()
{
    const After after = m_after;
    m_after = After::None;
    m_pendingAfter = After::None;
    setDialog(QString());
    m_document.markClean();
    proceed(after);
}

void Controller::dialogCancel()
{
    m_after = After::None;
    m_pendingAfter = After::None;
    setDialog(QString());
}

void Controller::closeError()
{
    if (m_pendingAfter != After::None) {
        m_after = m_pendingAfter;
        setDialog(QStringLiteral("save"));
        return;
    }
    setDialog(QString());
}

void Controller::escape()
{
    if (!m_dialog.isEmpty()) {
        if (m_dialog == QLatin1String("error")) {
            closeError();
        } else {
            dialogCancel();
        }
        return;
    }
    closeFind();
}

void Controller::loadSettings()
{
    const Settings::Values values = Settings::load();
    m_scheme = values.colorScheme;
    m_wordWrap = values.wordWrap;
    m_showStatusBar = values.showStatusBar;
    m_fontFamily = values.fontFamily;
    m_fontSize = values.fontSize;
    m_fontFamilyInput = m_fontFamily;
    m_fontSizeIndex = sizeIndex(m_fontSize);
}

void Controller::saveSettings() const
{
    Settings::Values values;
    values.colorScheme = m_scheme;
    values.wordWrap = m_wordWrap;
    values.showStatusBar = m_showStatusBar;
    values.fontFamily = m_fontFamily;
    values.fontSize = m_fontSize;
    Settings::save(values);
}

void Controller::applyScheme()
{
    QStyleHints *hints = QGuiApplication::styleHints();
    if (m_scheme == QLatin1String("light")) {
        hints->setColorScheme(Qt::ColorScheme::Light);
    } else if (m_scheme == QLatin1String("dark")) {
        hints->setColorScheme(Qt::ColorScheme::Dark);
    } else {
        hints->unsetColorScheme();
    }
}

void Controller::guard(After after)
{
    if (!m_document.isDirty()) {
        proceed(after);
        return;
    }
    m_after = after;
    setDialog(QStringLiteral("save"));
}

void Controller::proceed(After after)
{
    switch (after) {
    case After::NewDoc:
        m_document.reset();
        emit titleChanged();
        break;
    case After::Open:
        emit openDialogRequested();
        break;
    case After::OpenPath:
        loadPath(m_pendingPath);
        break;
    case After::Close:
        emit quitRequested();
        break;
    case After::None:
        break;
    }
}

void Controller::writeTo(const QString &path)
{
    const QString error = m_document.saveTo(path);
    if (!error.isEmpty()) {
        showError(error);
        return;
    }
    emit titleChanged();
    if (m_pendingAfter != After::None) {
        const After after = m_pendingAfter;
        m_pendingAfter = After::None;
        if (m_dialog == QLatin1String("save")) {
            setDialog(QString());
        }
        proceed(after);
    }
}

void Controller::loadPath(const QString &path)
{
    const QString error = m_document.loadPath(path);
    if (!error.isEmpty()) {
        showError(error);
        return;
    }
    emit titleChanged();
}

void Controller::showError(const QString &message)
{
    m_errorMessage = message;
    setDialog(QStringLiteral("error"));
}

void Controller::setDialog(const QString &dialog)
{
    if (m_dialog == dialog && dialog != QLatin1String("error")) {
        return;
    }
    m_dialog = dialog;
    emit dialogChanged();
}

void Controller::prefillFind()
{
    const QString selected = m_document.selectedText();
    if (!selected.isEmpty() && !selected.contains(u'\n')) {
        m_findText = selected;
    }
}

void Controller::insertText(const QString &text)
{
    const QString display = m_document.displayText();
    int from = qMin(m_document.selectionStart(), m_document.selectionEnd());
    int to = qMax(m_document.selectionStart(), m_document.selectionEnd());
    if (from == to) {
        from = m_document.cursorPosition();
        to = from;
    }
    const QString updated = display.left(from) + text + display.mid(to);
    const int cursor = from + text.size();
    m_document.applyDisplayText(updated, cursor, cursor, cursor);
}

QString Controller::firstPath(const QStringList &arguments, const QString &workingDirectory, bool skipProgram) const
{
    bool skippedProgram = !skipProgram;
    for (const QString &argument : arguments) {
        if (!skippedProgram) {
            skippedProgram = true;
            continue;
        }
        if (argument.isEmpty() || argument.startsWith(u'-')) {
            continue;
        }
        return absolutePath(argument, workingDirectory);
    }
    return {};
}
