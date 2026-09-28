// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "document.h"

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>

class Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Document *document READ document CONSTANT)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString dialog READ dialog NOTIFY dialogChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY dialogChanged)
    Q_PROPERTY(bool modalOpen READ modalOpen NOTIFY dialogChanged)
    Q_PROPERTY(bool fileDialogOpen READ fileDialogOpen WRITE setFileDialogOpen NOTIFY dialogChanged)
    Q_PROPERTY(bool findVisible READ findVisible NOTIFY findChanged)
    Q_PROPERTY(bool replaceVisible READ replaceVisible NOTIFY findChanged)
    Q_PROPERTY(QString findText READ findText WRITE setFindText NOTIFY findChanged)
    Q_PROPERTY(QString replaceText READ replaceText WRITE setReplaceText NOTIFY findChanged)
    Q_PROPERTY(bool matchCase READ matchCase WRITE setMatchCase NOTIFY findChanged)
    Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap NOTIFY appearanceChanged)
    Q_PROPERTY(bool showStatusBar READ showStatusBar WRITE setShowStatusBar NOTIFY appearanceChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily NOTIFY appearanceChanged)
    Q_PROPERTY(int fontSize READ fontSize NOTIFY appearanceChanged)
    Q_PROPERTY(QString fontFamilyInput READ fontFamilyInput WRITE setFontFamilyInput NOTIFY fontDialogChanged)
    Q_PROPERTY(int fontSizeIndex READ fontSizeIndex WRITE setFontSizeIndex NOTIFY fontDialogChanged)
    Q_PROPERTY(QStringList fontFamilies READ fontFamilies CONSTANT)
    Q_PROPERTY(QStringList fontSizes READ fontSizes CONSTANT)
    Q_PROPERTY(QString colorScheme READ colorScheme NOTIFY appearanceChanged)
    Q_PROPERTY(bool effectiveDark READ effectiveDark NOTIFY appearanceChanged)
    Q_PROPERTY(QString schemeButtonText READ schemeButtonText NOTIFY appearanceChanged)
    Q_PROPERTY(QString schemeButtonTooltip READ schemeButtonTooltip NOTIFY appearanceChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY cursorChanged)
    Q_PROPERTY(QString wrapLabel READ wrapLabel NOTIFY appearanceChanged)
    Q_PROPERTY(bool gotoEnabled READ gotoEnabled NOTIFY appearanceChanged)
    Q_PROPERTY(QString gotoInput READ gotoInput WRITE setGotoInput NOTIFY gotoChanged)
    Q_PROPERTY(QString gotoError READ gotoError NOTIFY gotoChanged)
    Q_PROPERTY(QUrl currentFolder READ currentFolder NOTIFY titleChanged)
    Q_PROPERTY(QUrl suggestedSaveUrl READ suggestedSaveUrl NOTIFY titleChanged)
    Q_PROPERTY(QVariantMap aboutData READ aboutData CONSTANT)

public:
    explicit Controller(QObject *parent = nullptr);

    Document *document();
    QString title() const;
    QString dialog() const;
    QString errorMessage() const;
    bool modalOpen() const;
    bool fileDialogOpen() const;
    void setFileDialogOpen(bool open);
    bool findVisible() const;
    bool replaceVisible() const;
    QString findText() const;
    void setFindText(const QString &text);
    QString replaceText() const;
    void setReplaceText(const QString &text);
    bool matchCase() const;
    void setMatchCase(bool match);
    bool wordWrap() const;
    void setWordWrap(bool enabled);
    bool showStatusBar() const;
    void setShowStatusBar(bool enabled);
    QString fontFamily() const;
    int fontSize() const;
    QString fontFamilyInput() const;
    void setFontFamilyInput(const QString &family);
    int fontSizeIndex() const;
    void setFontSizeIndex(int index);
    QStringList fontFamilies() const;
    QStringList fontSizes() const;
    QString colorScheme() const;
    bool effectiveDark() const;
    QString schemeButtonText() const;
    QString schemeButtonTooltip() const;
    QString statusText() const;
    QString wrapLabel() const;
    bool gotoEnabled() const;
    QString gotoInput() const;
    void setGotoInput(const QString &text);
    QString gotoError() const;
    QUrl currentFolder() const;
    QUrl suggestedSaveUrl() const;
    QVariantMap aboutData() const;

    void startup(const QString &path);
    Q_INVOKABLE void activate(const QStringList &arguments, const QString &workingDirectory);
    Q_INVOKABLE void activateUrls(const QList<QUrl> &urls);

    Q_INVOKABLE void newDocument();
    Q_INVOKABLE void open();
    Q_INVOKABLE void save();
    Q_INVOKABLE void saveAs();
    Q_INVOKABLE void requestClose();
    Q_INVOKABLE void loadChosen(const QUrl &url);
    Q_INVOKABLE void saveChosen(const QUrl &url);
    Q_INVOKABLE void fileDialogCancelled();

    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void cut();
    Q_INVOKABLE void copy();
    Q_INVOKABLE void paste();
    Q_INVOKABLE void deleteText();
    Q_INVOKABLE void selectAll();

    Q_INVOKABLE void showFind();
    Q_INVOKABLE void showReplace();
    Q_INVOKABLE void findNext();
    Q_INVOKABLE void replaceOne();
    Q_INVOKABLE void replaceAll();
    Q_INVOKABLE void closeFind();

    Q_INVOKABLE void goTo();
    Q_INVOKABLE void confirmGoTo();
    Q_INVOKABLE void insertDateTime();
    Q_INVOKABLE void showFontDialog();
    Q_INVOKABLE void applyFont();
    Q_INVOKABLE void setColorScheme(const QString &scheme);
    Q_INVOKABLE void toggleScheme();
    Q_INVOKABLE void showAbout();

    Q_INVOKABLE void dialogSave();
    Q_INVOKABLE void dialogDiscard();
    Q_INVOKABLE void dialogCancel();
    Q_INVOKABLE void closeError();
    Q_INVOKABLE void escape();

Q_SIGNALS:
    void titleChanged();
    void dialogChanged();
    void findChanged();
    void appearanceChanged();
    void cursorChanged();
    void fontDialogChanged();
    void gotoChanged();
    void openDialogRequested();
    void saveDialogRequested();
    void raiseRequested();
    void quitRequested();

private:
    enum class After { None, NewDoc, Open, OpenPath, Close };

    void loadSettings();
    void saveSettings() const;
    void applyScheme();
    void guard(After after);
    void proceed(After after);
    void writeTo(const QString &path);
    void loadPath(const QString &path);
    void showError(const QString &message);
    void setDialog(const QString &dialog);
    void prefillFind();
    void insertText(const QString &text);
    QString firstPath(const QStringList &arguments, const QString &workingDirectory, bool skipProgram) const;

    Document m_document;
    QString m_scheme = QStringLiteral("system");
    bool m_wordWrap = false;
    bool m_showStatusBar = true;
    QString m_fontFamily = QStringLiteral("monospace");
    int m_fontSize = 14;
    QString m_fontFamilyInput = QStringLiteral("monospace");
    int m_fontSizeIndex = 5;
    QString m_dialog;
    QString m_errorMessage;
    bool m_fileDialogOpen = false;
    bool m_findVisible = false;
    bool m_replaceVisible = false;
    QString m_findText;
    QString m_replaceText;
    bool m_matchCase = false;
    QString m_gotoInput = QStringLiteral("1");
    QString m_gotoError;
    After m_after = After::None;
    After m_pendingAfter = After::None;
    QString m_pendingPath;
};
