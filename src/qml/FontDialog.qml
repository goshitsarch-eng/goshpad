// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

QQC2.Dialog {
    id: dialog
    title: i18n("Font")
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 48 : 420, 420)
    standardButtons: QQC2.Dialog.NoButton
    closePolicy: QQC2.Popup.CloseOnEscape

    contentItem: ColumnLayout {
        spacing: 8
        QQC2.Label { text: i18n("Font family") }
        QQC2.ComboBox {
            id: familyCombo
            Layout.fillWidth: true
            model: controller.fontFamilies
            onActivated: controller.fontFamilyInput = currentText
        }
        QQC2.TextField {
            id: familyField
            Layout.fillWidth: true
            onTextEdited: controller.fontFamilyInput = text
        }
        QQC2.Label { text: i18n("Size") }
        QQC2.ComboBox {
            id: sizeCombo
            Layout.fillWidth: true
            model: controller.fontSizes
            onActivated: controller.fontSizeIndex = currentIndex
        }
    }

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            text: i18n("OK")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            onClicked: {
                controller.fontFamilyInput = familyField.text
                controller.applyFont()
            }
        }
        QQC2.Button {
            text: i18n("Cancel")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
            onClicked: controller.dialogCancel()
        }
    }

    onOpened: {
        familyField.text = controller.fontFamilyInput
        const index = controller.fontFamilies.indexOf(controller.fontFamilyInput)
        familyCombo.currentIndex = index
        sizeCombo.currentIndex = controller.fontSizeIndex
    }
}
