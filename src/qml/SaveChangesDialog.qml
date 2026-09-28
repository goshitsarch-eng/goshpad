// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

QQC2.Dialog {
    title: i18n("Save changes?")
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 48 : 420, 440)
    standardButtons: QQC2.Dialog.NoButton
    closePolicy: QQC2.Popup.CloseOnEscape

    contentItem: QQC2.Label {
        text: i18n("Your changes will be lost if you don't save them.")
        wrapMode: Text.WordWrap
    }

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            text: i18n("Save")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            onClicked: controller.dialogSave()
        }
        QQC2.Button {
            text: i18n("Discard")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.DestructiveRole
            onClicked: controller.dialogDiscard()
        }
        QQC2.Button {
            text: i18n("Cancel")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
            onClicked: controller.dialogCancel()
        }
    }
}
