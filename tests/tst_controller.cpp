// SPDX-License-Identifier: GPL-3.0-or-later

#include "controller.h"
#include "settings.h"

#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>

class ControllerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        Settings::save({});
    }

    void findNextSelectsThenWraps()
    {
        Controller controller;
        controller.document()->applyDisplayText(QStringLiteral("one two one"), 0, 0, 0);
        controller.setFindText(QStringLiteral("one"));
        controller.findNext();
        QCOMPARE(controller.document()->selectionStart(), 0);
        QCOMPARE(controller.document()->selectionEnd(), 3);
        controller.findNext();
        QCOMPARE(controller.document()->selectionStart(), 8);
        QCOMPARE(controller.document()->selectionEnd(), 11);
        controller.findNext();
        QCOMPARE(controller.document()->selectionStart(), 0);
        QCOMPARE(controller.document()->selectionEnd(), 3);
    }

    void replaceAllIsOneUndoStep()
    {
        Controller controller;
        controller.document()->applyDisplayText(QStringLiteral("one two one"), 0, 0, 0);
        const int undoBefore = controller.document()->undoCount();
        controller.setFindText(QStringLiteral("one"));
        controller.setReplaceText(QStringLiteral("ONE"));
        controller.replaceAll();
        QCOMPARE(controller.document()->displayText(), QStringLiteral("ONE two ONE"));
        QCOMPARE(controller.document()->undoCount(), undoBefore + 1);
    }

    void goToMovesCaretAndRejectsBadInput()
    {
        Controller controller;
        controller.document()->applyDisplayText(QStringLiteral("a\nb\nc"), 0, 0, 0);
        controller.goTo();
        QCOMPARE(controller.dialog(), QStringLiteral("goto"));
        controller.setGotoInput(QStringLiteral("2"));
        controller.confirmGoTo();
        QCOMPARE(controller.dialog(), QString());
        QCOMPARE(controller.document()->cursorPosition(), 2);
        controller.goTo();
        controller.setGotoInput(QStringLiteral("nope"));
        controller.confirmGoTo();
        QCOMPARE(controller.gotoError(), QStringLiteral("Please enter a valid line number."));
        controller.setGotoInput(QStringLiteral("9"));
        controller.confirmGoTo();
        QCOMPARE(controller.gotoError(), QStringLiteral("The line number is beyond the total number of lines."));
    }

    void goToIsDisabledWhileWordWrapIsOn()
    {
        Controller controller;
        controller.setWordWrap(true);
        controller.goTo();
        QCOMPARE(controller.dialog(), QString());
    }

    void saveFromPromptContinuesIntoNewDocument()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("note.txt"));

        Controller controller;
        controller.document()->applyDisplayText(QStringLiteral("hello"), 5, 5, 5);
        controller.newDocument();
        QCOMPARE(controller.dialog(), QStringLiteral("save"));
        QSignalSpy saveSpy(&controller, &Controller::saveDialogRequested);
        controller.dialogSave();
        QCOMPARE(saveSpy.count(), 1);
        controller.saveChosen(QUrl::fromLocalFile(path));
        QCOMPARE(controller.document()->displayText(), QString());
        QVERIFY(!controller.document()->isDirty());
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), QByteArrayLiteral("hello"));
    }

    void discardDropsChangesAndCancelKeepsThem()
    {
        Controller controller;
        controller.document()->applyDisplayText(QStringLiteral("hello"), 5, 5, 5);
        controller.newDocument();
        controller.dialogCancel();
        QCOMPARE(controller.dialog(), QString());
        QCOMPARE(controller.document()->displayText(), QStringLiteral("hello"));

        controller.newDocument();
        controller.dialogDiscard();
        QCOMPARE(controller.document()->displayText(), QString());
        QVERIFY(!controller.document()->isDirty());
    }

    void escapeClosesDialogThenFindBar()
    {
        Controller controller;
        controller.showFind();
        QVERIFY(controller.findVisible());
        controller.showAbout();
        QCOMPARE(controller.dialog(), QStringLiteral("about"));
        controller.escape();
        QCOMPARE(controller.dialog(), QString());
        QVERIFY(controller.findVisible());
        controller.escape();
        QVERIFY(!controller.findVisible());
    }

    void settingsRoundTrip()
    {
        {
            Controller controller;
            controller.setWordWrap(true);
            controller.setShowStatusBar(false);
            controller.setColorScheme(QStringLiteral("dark"));
            controller.setFontFamilyInput(QStringLiteral("DejaVu Sans Mono"));
            controller.setFontSizeIndex(controller.fontSizes().indexOf(QStringLiteral("18")));
            controller.applyFont();
        }
        Controller again;
        QCOMPARE(again.wordWrap(), true);
        QCOMPARE(again.showStatusBar(), false);
        QCOMPARE(again.colorScheme(), QStringLiteral("dark"));
        QCOMPARE(again.fontFamily(), QStringLiteral("DejaVu Sans Mono"));
        QCOMPARE(again.fontSize(), 18);
        QCOMPARE(again.effectiveDark(), true);
    }
};

QTEST_MAIN(ControllerTest)
#include "tst_controller.moc"
