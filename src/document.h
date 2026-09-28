// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QVector>

class Document : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString displayText READ displayText NOTIFY displayTextChanged)
    Q_PROPERTY(QString canonicalText READ canonicalText NOTIFY displayTextChanged)
    Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged)
    Q_PROPERTY(QString path READ path NOTIFY pathChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition NOTIFY cursorChanged)
    Q_PROPERTY(int selectionStart READ selectionStart NOTIFY cursorChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd NOTIFY cursorChanged)
    Q_PROPERTY(int line READ line NOTIFY cursorChanged)
    Q_PROPERTY(int column READ column NOTIFY cursorChanged)
    Q_PROPERTY(bool applying READ applying NOTIFY applyingChanged)
    Q_PROPERTY(int undoCount READ undoCount NOTIFY displayTextChanged)
    Q_PROPERTY(int redoCount READ redoCount NOTIFY displayTextChanged)

public:
    explicit Document(QObject *parent = nullptr);

    QString displayText() const;
    QString canonicalText() const;
    bool isDirty() const;
    QString path() const;
    int cursorPosition() const;
    int selectionStart() const;
    int selectionEnd() const;
    int line() const;
    int column() const;
    bool applying() const;
    int undoCount() const;
    int redoCount() const;
    QString selectedText() const;
    bool hasSelection() const;

    Q_INVOKABLE QString loadPath(const QString &path);
    Q_INVOKABLE QString saveTo(const QString &path);
    Q_INVOKABLE void reset();
    Q_INVOKABLE void markClean();
    Q_INVOKABLE void applyDisplayText(const QString &display, int cursor, int selStart, int selEnd);
    Q_INVOKABLE void noteCursor(int cursor, int selStart, int selEnd);
    Q_INVOKABLE void commitCursor(int cursor, int selStart, int selEnd);
    Q_INVOKABLE bool undo();
    Q_INVOKABLE bool redo();
    Q_INVOKABLE void selectRange(int from, int to);

Q_SIGNALS:
    void displayTextChanged();
    void dirtyChanged();
    void pathChanged();
    void cursorChanged();
    void applyingChanged();

private:
    struct Snapshot {
        QString canonical;
        int cursor = 0;
        int selStart = 0;
        int selEnd = 0;
    };

    void pushUndo();
    void restore(const Snapshot &snapshot);
    void setCursorState(int cursor, int selStart, int selEnd);
    static QString toDisplay(const QString &canonical);
    static QString merge(const QString &canonical, const QString &display);

    QString m_canonical;
    QString m_saved;
    QString m_path;
    int m_cursor = 0;
    int m_selStart = 0;
    int m_selEnd = 0;
    int m_undoCursor = 0;
    int m_undoSelStart = 0;
    int m_undoSelEnd = 0;
    bool m_applying = false;
    QVector<Snapshot> m_undo;
    QVector<Snapshot> m_redo;
};
