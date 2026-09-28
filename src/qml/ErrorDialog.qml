// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

QQC2.Dialog {
    title: i18n("Error")
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 48 : 440, 480)
    standardButtons: QQC2.Dialog.Ok
    closePolicy: QQC2.Popup.CloseOnEscape

    contentItem: QQC2.Label {
        text: controller.errorMessage
        wrapMode: Text.WordWrap
    }

    onAccepted: controller.closeError()
}
