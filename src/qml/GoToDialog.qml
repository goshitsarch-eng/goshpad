// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

QQC2.Dialog {
    id: dialog
    title: i18n("Go To Line")
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 48 : 360, 360)
    standardButtons: QQC2.Dialog.NoButton
    closePolicy: QQC2.Popup.CloseOnEscape

    contentItem: ColumnLayout {
        spacing: 8
        QQC2.Label { text: i18n("Line number") }
        QQC2.TextField {
            id: lineField
            Layout.fillWidth: true
            onTextEdited: controller.gotoInput = text
            onAccepted: controller.confirmGoTo()
        }
        QQC2.Label {
            visible: controller.gotoError.length > 0
            text: controller.gotoError
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            color: Kirigami.Theme.negativeTextColor
        }
    }

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            text: i18n("Go To")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            onClicked: controller.confirmGoTo()
        }
        QQC2.Button {
            text: i18n("Cancel")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
            onClicked: controller.dialogCancel()
        }
    }

    onOpened: {
        lineField.text = controller.gotoInput
        lineField.forceActiveFocus()
        lineField.selectAll()
    }

}
