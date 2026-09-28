// SPDX-License-Identifier: GPL-3.0-or-later

#include "textops.h"

#include <QTest>

#include <optional>
#include <string>

namespace {

std::string slice(const std::string &text, textops::Range range)
{
    return text.substr(range.start, range.end - range.start);
}

} // namespace

class TextOpsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void findsFirstMatchFromStart()
    {
        const auto found = textops::find_next("one two one", 0, "one", false, true);
        QVERIFY(found.has_value());
        QCOMPARE(found->start, std::size_t(0));
        QCOMPARE(found->end, std::size_t(3));
    }

    void skipsCurrentSelectionToNextMatch()
    {
        const auto first = textops::find_next("one two one", 0, "one", false, true);
        QVERIFY(first.has_value());
        const auto second = textops::find_next("one two one", first->end, "one", false, true);
        QVERIFY(second.has_value());
        QCOMPARE(second->start, std::size_t(8));
        QCOMPARE(second->end, std::size_t(11));
    }

    void wrapsAroundToFirstMatch()
    {
        const std::string text = "one two one";
        const auto first = textops::find_next(text, 0, "one", false, true);
        const auto second = textops::find_next(text, first->end, "one", false, true);
        const auto wrapped = textops::find_next(text, second->end, "one", false, true);
        QCOMPARE(wrapped->start, std::size_t(0));
        QCOMPARE(wrapped->end, std::size_t(3));
    }

    void caseInsensitiveByDefault()
    {
        const std::string text = "Hello HELLO";
        const auto found = textops::find_next(text, 0, "hello", false, true);
        QVERIFY(found.has_value());
        QCOMPARE(slice(text, *found), std::string("Hello"));
    }

    void matchCase()
    {
        const std::string text = "Hello hello";
        const auto found = textops::find_next(text, 0, "hello", true, true);
        QVERIFY(found.has_value());
        QCOMPARE(found->start, std::size_t(6));
        QCOMPARE(found->end, std::size_t(11));
        QCOMPARE(slice(text, *found), std::string("hello"));
    }

    void missingTextReturnsNone()
    {
        QVERIFY(!textops::find_next("hello", 0, "xyz", false, true));
        QVERIFY(!textops::find_next("hello", 0, "", false, true));
    }

    void replaceCurrentThenFindNext()
    {
        const auto first = textops::find_next("one two one", 0, "one", false, true);
        const auto result = textops::replace_and_find_next("one two one", first, "one", "ONE", false, true);
        QVERIFY(result.has_value());
        QCOMPARE(result->kind, textops::ReplaceKind::Replaced);
        QCOMPARE(result->text, std::string("ONE two one"));
        QVERIFY(result->next.has_value());
        QCOMPARE(result->next->start, std::size_t(8));
        QCOMPARE(result->next->end, std::size_t(11));
        QCOMPARE(result->text.substr(result->next->start, 3), std::string("one"));
    }

    void replaceFindsFirstIfNothingSelected()
    {
        const auto result = textops::replace_and_find_next("one two one", std::nullopt, "one", "ONE", false, true);
        QVERIFY(result.has_value());
        QCOMPARE(result->kind, textops::ReplaceKind::Found);
        QCOMPARE(result->next->start, std::size_t(0));
        QCOMPARE(result->next->end, std::size_t(3));
    }

    void replaceAllMatches()
    {
        const auto [text, count] = textops::replace_all("one two one two one", "one", "ONE", false);
        QCOMPARE(count, std::size_t(3));
        QCOMPARE(text, std::string("ONE two ONE two ONE"));
    }

    void replaceAllIsCaseSensitiveWhenAsked()
    {
        const auto [text, count] = textops::replace_all("One one ONE", "one", "x", true);
        QCOMPARE(count, std::size_t(1));
        QCOMPARE(text, std::string("One x ONE"));
    }

    void replaceAllEmptyNeedleIsNoop()
    {
        const auto [text, count] = textops::replace_all("abc", "", "x", false);
        QCOMPARE(count, std::size_t(0));
        QCOMPARE(text, std::string("abc"));
    }

    void selectionMatchesIgnoresCaseByDefault()
    {
        QVERIFY(textops::selection_matches("Hello", "hello", false));
        QVERIFY(!textops::selection_matches("Hello", "hello", true));
    }

    void goesToRequestedLine()
    {
        const auto offset = textops::goto_line("a\nb\nc", 2);
        QVERIFY(offset.has_value());
        QCOMPARE(*offset, std::size_t(2));
        const auto position = textops::line_col_at("a\nb\nc", *offset);
        QCOMPARE(position.first, std::size_t(2));
        QCOMPARE(position.second, std::size_t(1));
    }

    void rejectsOutOfRange()
    {
        QVERIFY(!textops::goto_line("a\nb", 0));
        QVERIFY(!textops::goto_line("a\nb", 3));
        QVERIFY(textops::goto_line("a\nb", 2).has_value());
    }

    void rejectsFarOutOfRange()
    {
        QVERIFY(!textops::goto_line("a\nb", 100));
    }

    void lineColAtSnapsMidUtf8ToPreviousBoundary()
    {
        const std::string text = "é\nx";
        QCOMPARE(static_cast<unsigned char>(text[0]), static_cast<unsigned char>(0xc3));
        QCOMPARE(textops::line_col_at(text, 1), (std::pair<std::size_t, std::size_t>{1, 1}));
        QCOMPARE(textops::line_col_at(text, 2), (std::pair<std::size_t, std::size_t>{1, 2}));
        QCOMPARE(textops::line_col_at(text, 3), (std::pair<std::size_t, std::size_t>{2, 1}));
    }

    void offsetAtLineColStaysOnRequestedLine()
    {
        const std::string text = "ab\ncd";
        QCOMPARE(textops::offset_at_line_col(text, 0, 0), std::size_t(0));
        QCOMPARE(textops::offset_at_line_col(text, 0, 2), std::size_t(2));
        QCOMPARE(textops::offset_at_line_col(text, 0, 99), std::size_t(2));
        QCOMPARE(textops::offset_at_line_col(text, 1, 0), std::size_t(3));
        QCOMPARE(textops::offset_at_line_col(text, 1, 99), std::size_t(5));
    }

    void findFromSnapsForwardFromMidUtf8Start()
    {
        const std::string text = "éabc";
        const auto found = textops::find_from(text, "abc", 1, true);
        QVERIFY(found.has_value());
        QCOMPARE(found->start, std::size_t(2));
        QCOMPARE(found->end, std::size_t(5));
        QVERIFY(!textops::find_from(text, "abc", text.size() + 1, true));
    }

    void lineColAndFindHandleCjk()
    {
        const std::string text = "日本語\nnext";
        QCOMPARE(textops::line_col_at(text, 0), (std::pair<std::size_t, std::size_t>{1, 1}));
        QCOMPARE(textops::line_col_at(text, std::string("日").size()), (std::pair<std::size_t, std::size_t>{1, 2}));
        QCOMPARE(textops::line_col_at(text, std::string("日本語").size()), (std::pair<std::size_t, std::size_t>{1, 4}));
        QCOMPARE(textops::line_col_at(text, 1), (std::pair<std::size_t, std::size_t>{1, 1}));
        const auto found = textops::find_next(text, 1, "next", true, true);
        QVERIFY(found.has_value());
        QCOMPARE(found->start, std::string("日本語\n").size());
        QCOMPARE(found->end, text.size());
        QCOMPARE(textops::offset_at_line_col(text, 0, 99), std::string("日本語").size());
        QCOMPARE(textops::offset_at_line_col(text, 1, 0), std::string("日本語\n").size());
    }

    void caretLineColMatchesTwoStepConversion()
    {
        const std::string corpus[] = {
            "",
            "a",
            "ab\ncd",
            "ab\ncd\n",
            "\n\n",
            "a\n",
            "日本語\nnext",
            "é\nx",
            "a\r\nb",
            "line1\nline2\nline3",
            "trailing spaces  \n\tindented",
        };
        for (const std::string &text : corpus) {
            for (std::size_t line = 0; line < 6; ++line) {
                for (std::size_t column = 0; column < 10; ++column) {
                    const auto offset = textops::offset_at_line_col(text, line, column);
                    const auto expected = textops::line_col_at(text, offset);
                    QCOMPARE(textops::caret_line_col(text, line, column), expected);
                }
            }
        }
    }

    void findNextWrapsAfterLastMatch()
    {
        const std::string text = "日本語 one 日本語";
        const auto first = textops::find_next(text, 0, "日本語", true, true);
        QVERIFY(first.has_value());
        QCOMPARE(first->start, std::size_t(0));
        QCOMPARE(first->end, std::string("日本語").size());
        const auto second = textops::find_next(text, first->end, "日本語", true, true);
        QVERIFY(second.has_value());
        QCOMPARE(slice(text, *second), std::string("日本語"));
        const auto wrapped = textops::find_next(text, second->end, "日本語", true, true);
        QCOMPARE(wrapped->start, first->start);
        QCOMPARE(wrapped->end, first->end);
    }
};

QTEST_GUILESS_MAIN(TextOpsTest)
#include "tst_textops.moc"
