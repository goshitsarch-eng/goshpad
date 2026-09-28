// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.Dialog {
    id: dialog
    title: i18n("About GoshPad")
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 32 : 480, 480)
    height: Math.min(parent ? parent.height - 32 : 560, 560)
    standardButtons: QQC2.Dialog.Close
    closePolicy: QQC2.Popup.CloseOnEscape

    contentItem: Loader {
        active: dialog.opened
        sourceComponent: Kirigami.ScrollablePage {
            Kirigami.AboutItem {
                aboutData: controller.aboutData
                implicitWidth: dialog.availableWidth
            }
        }
    }
}
