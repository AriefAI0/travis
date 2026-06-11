import QtQuick
import QtQuick.Layouts

import "../.."

// Hosts the native preview surface in the center inspection canvas.

Rectangle {
    id: root

    property var previewController

    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#000000"

    PreviewSurface {
        anchors.fill: parent
        previewController: root.previewController
    }
}
