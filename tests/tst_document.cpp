// SPDX-License-Identifier: GPL-3.0-or-later

#include "document.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

class DocumentTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void invalidUtf8LeavesDocumentUntouched()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("bad.txt"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        const QByteArray bytes = QByteArrayLiteral("foo") + QByteArray::fromRawData("\xff\xfe", 2);
        QCOMPARE(file.write(bytes), qint64(bytes.size()));
        file.close();

        Document document;
        document.applyDisplayText(QStringLiteral("existing doc"), 12, 12, 12);
        document.applyDisplayText(QStringLiteral("existing doc one"), 16, 16, 16);
        QVERIFY(document.undo());
        QCOMPARE(document.undoCount(), 1);
        QCOMPARE(document.redoCount(), 1);
        const QString canonical = document.canonicalText();
        const QString savedDisplay = document.displayText();

        const QString error = document.loadPath(path);
        QVERIFY(error.startsWith(QStringLiteral("Could not open file:")));
        QVERIFY(error.contains(path));
        QVERIFY(error.toLower().contains(QStringLiteral("utf-8")));
        QCOMPARE(document.displayText(), savedDisplay);
        QCOMPARE(document.canonicalText(), canonical);
        QCOMPARE(document.path(), QString());
        QCOMPARE(document.undoCount(), 1);
        QCOMPARE(document.redoCount(), 1);
    }

    void missingFileNamesThePath()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("missing.txt"));
        Document document;
        document.applyDisplayText(QStringLiteral("existing doc"), 12, 12, 12);
        const QString error = document.loadPath(path);
        QVERIFY(error.startsWith(QStringLiteral("Could not open file:")));
        QVERIFY(error.contains(path));
        QVERIFY(error.contains(QStringLiteral("No such file or directory")));
        QCOMPARE(document.displayText(), QStringLiteral("existing doc"));
        QCOMPARE(document.path(), QString());
    }

    void crlfRoundTripIsByteIdentical()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("crlf.txt"));
        const QByteArray bytes = QByteArrayLiteral("a\r\nb\r\n");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(bytes), qint64(bytes.size()));
        file.close();

        Document document;
        QVERIFY(document.loadPath(path).isEmpty());
        QCOMPARE(document.canonicalText(), QStringLiteral("a\r\nb\r\n"));
        QCOMPARE(document.displayText(), QStringLiteral("a\nb\n"));
        QCOMPARE(document.path(), path);
        QVERIFY(!document.isDirty());

        QVERIFY(document.saveTo(path).isEmpty());
        QFile saved(path);
        QVERIFY(saved.open(QIODevice::ReadOnly));
        QCOMPARE(saved.readAll(), bytes);
        QVERIFY(!document.isDirty());
    }

    void mixedEndingsRoundTrip()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("mixed.txt"));
        const QByteArray bytes = QByteArrayLiteral("a\r\nb\nc");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(bytes), qint64(bytes.size()));
        file.close();

        Document document;
        QVERIFY(document.loadPath(path).isEmpty());
        QCOMPARE(document.canonicalText(), QStringLiteral("a\r\nb\nc"));
        QVERIFY(document.saveTo(path).isEmpty());
        QFile saved(path);
        QVERIFY(saved.open(QIODevice::ReadOnly));
        QCOMPARE(saved.readAll(), bytes);
        QVERIFY(!document.isDirty());
    }

    void loadSuccessClearsUndoAndSetsTitlePieces()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("note.txt"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("hello world");
        file.close();

        Document document;
        document.applyDisplayText(QStringLiteral("stale content"), 13, 13, 13);
        QVERIFY(document.undoCount() > 0);
        QVERIFY(document.loadPath(path).isEmpty());
        QCOMPARE(document.displayText(), QStringLiteral("hello world"));
        QCOMPARE(document.canonicalText(), QStringLiteral("hello world"));
        QCOMPARE(document.path(), path);
        QVERIFY(!document.isDirty());
        QCOMPARE(document.undoCount(), 0);
        QCOMPARE(document.redoCount(), 0);
        QVERIFY(QFileInfo(path).fileName() == QStringLiteral("note.txt"));
    }

    void withinLineEditPreservesCrlf()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("crlf.txt"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("a\r\nb\r\n");
        file.close();

        Document document;
        QVERIFY(document.loadPath(path).isEmpty());
        document.applyDisplayText(QStringLiteral("aX\nb\n"), 2, 2, 2);
        QCOMPARE(document.canonicalText(), QStringLiteral("aX\r\nb\r\n"));
        QVERIFY(document.isDirty());
        QVERIFY(document.saveTo(path).isEmpty());
        QFile saved(path);
        QVERIFY(saved.open(QIODevice::ReadOnly));
        QCOMPARE(saved.readAll(), QByteArrayLiteral("aX\r\nb\r\n"));
    }

    void insertedLineUsesLf()
    {
        Document document;
        document.applyDisplayText(QStringLiteral("a\nb\n"), 0, 0, 0);
        document.markClean();
        const QString canonical = QStringLiteral("a\r\nb\r\n");
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("crlf.txt"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(canonical.toUtf8());
        file.close();
        QVERIFY(document.loadPath(path).isEmpty());
        document.applyDisplayText(QStringLiteral("a\nX\nb\n"), 3, 3, 3);
        QCOMPARE(document.canonicalText(), QStringLiteral("a\r\nX\nb\r\n"));
    }

    void undoRestoresCaretAndCapsAt100()
    {
        Document document;
        document.commitCursor(0, 0, 0);
        for (int index = 1; index <= 105; ++index) {
            const QString text(index, QLatin1Char('a'));
            document.applyDisplayText(text, index, index, index);
        }
        QCOMPARE(document.undoCount(), 100);
        QVERIFY(document.undo());
        QCOMPARE(document.displayText().size(), 104);
        QCOMPARE(document.cursorPosition(), 104);
        QVERIFY(document.redo());
        QCOMPARE(document.displayText().size(), 105);
        QCOMPARE(document.cursorPosition(), 105);
        document.applyDisplayText(QStringLiteral("z"), 1, 1, 1);
        QCOMPARE(document.redoCount(), 0);
    }
};

QTEST_GUILESS_MAIN(DocumentTest)
#include "tst_document.moc"
