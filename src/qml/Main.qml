// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    title: controller.title
    width: 820
    height: 600
    minimumWidth: 360
    minimumHeight: 180

    property bool allowClose: false

    pageStack.initialPage: editorPage

    Kirigami.Page {
        id: editorPage
        padding: 0
        title: controller.title

        actions: [
            Kirigami.Action {
                text: controller.schemeButtonText
                tooltip: controller.schemeButtonTooltip
                icon.name: controller.effectiveDark ? "brightness-high" : "brightness-low"
                onTriggered: controller.toggleScheme()
            }
        ]

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            QQC2.MenuBar {
                id: menuBar
                Layout.fillWidth: true

                QQC2.Menu {
                    title: i18n("File")
                    QQC2.MenuItem { action: newAction }
                    QQC2.MenuItem { action: openAction }
                    QQC2.MenuItem { action: saveAction }
                    QQC2.MenuItem { action: saveAsAction }
                    QQC2.MenuSeparator {}
                    QQC2.MenuItem { action: exitAction }
                }
                QQC2.Menu {
                    title: i18n("Edit")
                    QQC2.MenuItem { action: undoAction }
                    QQC2.MenuItem { action: redoAction }
                    QQC2.MenuSeparator {}
                    QQC2.MenuItem { action: cutAction }
                    QQC2.MenuItem { action: copyAction }
                    QQC2.MenuItem { action: pasteAction }
                    QQC2.MenuItem { action: deleteAction }
                    QQC2.MenuSeparator {}
                    QQC2.MenuItem { action: findAction }
                    QQC2.MenuItem { action: findNextAction }
                    QQC2.MenuItem { action: replaceAction }
                    QQC2.MenuItem { action: gotoAction }
                    QQC2.MenuSeparator {}
                    QQC2.MenuItem { action: selectAllAction }
                    QQC2.MenuItem { action: dateAction }
                }
                QQC2.Menu {
                    title: i18n("Format")
                    QQC2.MenuItem { action: wrapAction }
                    QQC2.MenuItem { action: fontAction }
                }
                QQC2.Menu {
                    title: i18n("View")
                    QQC2.MenuItem { action: statusAction }
                    QQC2.Menu {
                        title: i18n("Color Scheme")
                        QQC2.MenuItem { action: systemSchemeAction }
                        QQC2.MenuItem { action: lightSchemeAction }
                        QQC2.MenuItem { action: darkSchemeAction }
                    }
                }
                QQC2.Menu {
                    title: i18n("Help")
                    QQC2.MenuItem { action: aboutAction }
                }
            }

            ColumnLayout {
                id: findBar
                visible: controller.findVisible
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                RowLayout {
                    Layout.fillWidth: true
                    Layout.margins: Kirigami.Units.smallSpacing
                    QQC2.ToolButton {
                        text: "✕"
                        onClicked: controller.closeFind()
                        Accessible.name: i18n("Close Find (Esc)")
                    }
                    QQC2.TextField {
                        id: findField
                        Layout.fillWidth: true
                        placeholderText: i18n("Find")
                        onTextEdited: controller.findText = text
                        onAccepted: controller.findNext()
                    }
                    QQC2.CheckBox {
                        id: matchCaseBox
                        text: i18n("Match case")
                        onToggled: controller.matchCase = checked
                    }
                    QQC2.Button {
                        text: i18n("Find Next")
                        onClicked: controller.findNext()
                    }
                }
                RowLayout {
                    visible: controller.replaceVisible
                    Layout.fillWidth: true
                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                    QQC2.TextField {
                        id: replaceField
                        Layout.fillWidth: true
                        placeholderText: i18n("Replace with")
                        onTextEdited: controller.replaceText = text
                        onAccepted: controller.replaceOne()
                    }
                    QQC2.Button {
                        text: i18n("Replace")
                        onClicked: controller.replaceOne()
                    }
                    QQC2.Button {
                        text: i18n("Replace All")
                        onClicked: controller.replaceAll()
                    }
                }
            }

            QQC2.TextArea {
                id: editor
                Layout.fillWidth: true
                Layout.fillHeight: true
                selectByMouse: true
                persistentSelection: true
                wrapMode: controller.wordWrap ? TextEdit.Wrap : TextEdit.NoWrap
                font.family: controller.fontFamily
                font.pointSize: controller.fontSize
                background: Rectangle { color: Kirigami.Theme.backgroundColor }

                property bool syncing: false

                function pullFromDocument() {
                    syncing = true
                    const next = controller.document.displayText
                    if (text !== next)
                        text = next
                    const start = controller.document.selectionStart
                    const end = controller.document.selectionEnd
                    select(Math.min(start, end), Math.max(start, end))
                    cursorPosition = controller.document.cursorPosition
                    syncing = false
                }

                function rememberCaret() {
                    if (syncing || controller.document.applying)
                        return
                    const pos = cursorPosition
                    const start = selectionStart
                    const end = selectionEnd
                    controller.document.noteCursor(pos, start, end)
                    Qt.callLater(function() {
                        if (!syncing && !controller.document.applying)
                            controller.document.commitCursor(pos, start, end)
                    })
                }

                onTextChanged: {
                    if (syncing || controller.document.applying)
                        return
                    controller.document.applyDisplayText(text, cursorPosition, selectionStart, selectionEnd)
                }
                onCursorPositionChanged: rememberCaret()
                onSelectedTextChanged: rememberCaret()

                TapHandler {
                    acceptedButtons: Qt.RightButton
                    onPressedChanged: {
                        if (pressed)
                            contextMenu.popup()
                    }
                }
            }

            RowLayout {
                visible: controller.showStatusBar
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing
                QQC2.Label { text: controller.wrapLabel }
                Item { Layout.fillWidth: true }
                QQC2.Label { text: controller.statusText }
            }
        }
    }

    QQC2.Action {
        id: newAction
        text: i18n("New")
        shortcut: "Ctrl+N"
        enabled: !controller.modalOpen
        onTriggered: controller.newDocument()
    }
    QQC2.Action {
        id: openAction
        text: i18n("Open…")
        shortcut: "Ctrl+O"
        enabled: !controller.modalOpen
        onTriggered: controller.open()
    }
    QQC2.Action {
        id: saveAction
        text: i18n("Save")
        shortcut: "Ctrl+S"
        enabled: !controller.modalOpen
        onTriggered: controller.save()
    }
    QQC2.Action {
        id: saveAsAction
        text: i18n("Save As…")
        shortcut: "Ctrl+Shift+S"
        enabled: !controller.modalOpen
        onTriggered: controller.saveAs()
    }
    QQC2.Action {
        id: exitAction
        text: i18n("Exit")
        shortcut: "Ctrl+Q"
        enabled: !controller.modalOpen
        onTriggered: controller.requestClose()
    }
    QQC2.Action {
        id: undoAction
        text: i18n("Undo")
        shortcut: "Ctrl+Z"
        enabled: !controller.modalOpen && controller.document.undoCount > 0
        onTriggered: controller.undo()
    }
    QQC2.Action {
        id: redoAction
        text: i18n("Redo")
        shortcut: "Ctrl+Y"
        enabled: !controller.modalOpen && controller.document.redoCount > 0
        onTriggered: controller.redo()
    }
    Shortcut {
        sequence: "Ctrl+Shift+Z"
        enabled: !controller.modalOpen && controller.document.redoCount > 0
        context: Qt.ApplicationShortcut
        onActivated: controller.redo()
    }
    QQC2.Action { id: cutAction; text: i18n("Cut"); onTriggered: controller.cut() }
    QQC2.Action { id: copyAction; text: i18n("Copy"); onTriggered: controller.copy() }
    QQC2.Action { id: pasteAction; text: i18n("Paste"); onTriggered: controller.paste() }
    QQC2.Action { id: deleteAction; text: i18n("Delete"); onTriggered: controller.deleteText() }
    Shortcut {
        sequence: "Ctrl+X"
        enabled: editor.activeFocus && !controller.modalOpen
        context: Qt.WindowShortcut
        onActivated: controller.cut()
    }
    Shortcut {
        sequence: "Ctrl+C"
        enabled: editor.activeFocus && !controller.modalOpen
        context: Qt.WindowShortcut
        onActivated: controller.copy()
    }
    Shortcut {
        sequence: "Ctrl+V"
        enabled: editor.activeFocus && !controller.modalOpen
        context: Qt.WindowShortcut
        onActivated: controller.paste()
    }
    Shortcut {
        sequence: "Ctrl+A"
        enabled: editor.activeFocus && !controller.modalOpen
        context: Qt.WindowShortcut
        onActivated: controller.selectAll()
    }
    Shortcut {
        sequence: "Shift+Delete"
        enabled: editor.activeFocus && !controller.modalOpen
        context: Qt.WindowShortcut
        onActivated: controller.cut()
    }
    Shortcut {
        sequence: "Ctrl+Insert"
        enabled: editor.activeFocus && !controller.modalOpen
        context: Qt.WindowShortcut
        onActivated: controller.copy()
    }
    Shortcut {
        sequence: "Shift+Insert"
        enabled: editor.activeFocus && !controller.modalOpen
        context: Qt.WindowShortcut
        onActivated: controller.paste()
    }
    QQC2.Action {
        id: findAction
        text: i18n("Find…")
        shortcut: "Ctrl+F"
        enabled: !controller.modalOpen
        onTriggered: controller.showFind()
    }
    QQC2.Action {
        id: findNextAction
        text: i18n("Find Next")
        shortcut: "F3"
        enabled: !controller.modalOpen
        onTriggered: controller.findNext()
    }
    QQC2.Action {
        id: replaceAction
        text: i18n("Replace…")
        shortcut: "Ctrl+H"
        enabled: !controller.modalOpen
        onTriggered: controller.showReplace()
    }
    QQC2.Action {
        id: gotoAction
        text: i18n("Go To…")
        shortcut: "Ctrl+G"
        enabled: !controller.modalOpen && controller.gotoEnabled
        onTriggered: controller.goTo()
    }
    QQC2.Action {
        id: selectAllAction
        text: i18n("Select All")
        onTriggered: controller.selectAll()
    }
    QQC2.Action {
        id: dateAction
        text: i18n("Time/Date")
        shortcut: "F5"
        enabled: !controller.modalOpen
        onTriggered: controller.insertDateTime()
    }
    QQC2.Action {
        id: wrapAction
        text: i18n("Word Wrap")
        checkable: true
        checked: controller.wordWrap
        onCheckedChanged: {
            if (controller.wordWrap !== checked)
                controller.wordWrap = checked
        }
    }
    QQC2.Action {
        id: fontAction
        text: i18n("Font…")
        enabled: !controller.modalOpen
        onTriggered: controller.showFontDialog()
    }
    QQC2.Action {
        id: statusAction
        text: i18n("Status Bar")
        checkable: true
        checked: controller.showStatusBar
        onCheckedChanged: {
            if (controller.showStatusBar !== checked)
                controller.showStatusBar = checked
        }
    }
    QQC2.Action {
        id: systemSchemeAction
        text: i18n("System")
        checkable: true
        checked: controller.colorScheme === "system"
        onTriggered: controller.setColorScheme("system")
    }
    QQC2.Action {
        id: lightSchemeAction
        text: i18n("Light")
        checkable: true
        checked: controller.colorScheme === "light"
        onTriggered: controller.setColorScheme("light")
    }
    QQC2.Action {
        id: darkSchemeAction
        text: i18n("Dark")
        checkable: true
        checked: controller.colorScheme === "dark"
        onTriggered: controller.setColorScheme("dark")
    }
    QQC2.Action {
        id: aboutAction
        text: i18n("About GoshPad")
        enabled: !controller.modalOpen
        onTriggered: controller.showAbout()
    }
    Shortcut {
        sequence: "Esc"
        context: Qt.ApplicationShortcut
        onActivated: controller.escape()
    }

    QQC2.Menu {
        id: contextMenu
        QQC2.MenuItem { action: undoAction }
        QQC2.MenuItem { action: redoAction }
        QQC2.MenuSeparator {}
        QQC2.MenuItem { action: cutAction }
        QQC2.MenuItem { action: copyAction }
        QQC2.MenuItem { action: pasteAction }
        QQC2.MenuItem { action: deleteAction }
        QQC2.MenuSeparator {}
        QQC2.MenuItem { action: selectAllAction }
    }

    SaveChangesDialog {
        id: saveDialog
        onRejected: {
            if (controller.dialog === "save")
                controller.dialogCancel()
        }
    }
    GoToDialog {
        id: gotoDialog
        onRejected: {
            if (controller.dialog === "goto")
                controller.dialogCancel()
        }
    }
    FontDialog {
        id: fontDialog
        onRejected: {
            if (controller.dialog === "font")
                controller.dialogCancel()
        }
    }
    ErrorDialog {
        id: errorDialog
        onRejected: {
            if (controller.dialog === "error")
                controller.closeError()
        }
    }
    AboutDialog {
        id: aboutDialog
        onRejected: {
            if (controller.dialog === "about")
                controller.dialogCancel()
        }
    }

    FileDialog {
        id: openDialog
        title: i18n("Open")
        fileMode: FileDialog.OpenFile
        nameFilters: [i18n("Text files (*.txt)"), i18n("All files (*)")]
        onAccepted: controller.loadChosen(selectedFile)
        onRejected: controller.fileDialogCancelled()
    }
    FileDialog {
        id: saveDialogFile
        title: i18n("Save As")
        fileMode: FileDialog.SaveFile
        nameFilters: [i18n("Text files (*.txt)"), i18n("All files (*)")]
        onAccepted: controller.saveChosen(selectedFile)
        onRejected: controller.fileDialogCancelled()
    }

    function syncPopup(popup, name) {
        if (controller.dialog === name) {
            if (!popup.opened)
                popup.open()
        } else if (popup.opened) {
            popup.close()
        }
    }

    Connections {
        target: controller
        function onDialogChanged() {
            root.syncPopup(saveDialog, "save")
            root.syncPopup(gotoDialog, "goto")
            root.syncPopup(fontDialog, "font")
            root.syncPopup(errorDialog, "error")
            root.syncPopup(aboutDialog, "about")
        }
        function onFindChanged() {
            if (findField.text !== controller.findText)
                findField.text = controller.findText
            if (replaceField.text !== controller.replaceText)
                replaceField.text = controller.replaceText
            if (matchCaseBox.checked !== controller.matchCase)
                matchCaseBox.checked = controller.matchCase
        }
        function onOpenDialogRequested() {
            openDialog.currentFolder = controller.currentFolder
            controller.fileDialogOpen = true
            openDialog.open()
        }
        function onSaveDialogRequested() {
            saveDialogFile.currentFolder = controller.currentFolder
            saveDialogFile.currentFile = controller.suggestedSaveUrl
            controller.fileDialogOpen = true
            saveDialogFile.open()
        }
        function onRaiseRequested() {
            root.show()
            root.raise()
            root.requestActivate()
        }
        function onQuitRequested() {
            root.allowClose = true
            root.close()
        }
    }

    Connections {
        target: controller.document
        function onDisplayTextChanged() {
            editor.pullFromDocument()
        }
        function onCursorChanged() {
            if (editor.syncing || controller.document.applying)
                return
            if (editor.cursorPosition !== controller.document.cursorPosition
                    || editor.selectionStart !== controller.document.selectionStart
                    || editor.selectionEnd !== controller.document.selectionEnd)
                editor.pullFromDocument()
        }
    }

    Component.onCompleted: {
        editor.pullFromDocument()
        editor.forceActiveFocus()
    }

    onClosing: close => {
        if (root.allowClose)
            return
        close.accepted = false
        controller.requestClose()
    }
}
