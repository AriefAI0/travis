import QtQuick

// Wraps the Qt-owned preview item and binds it to the preview controller automatically.

Rectangle {
    id: root

    property var previewController

    color: "#0f141a"
    border.color: "#3a4652"
    border.width: 1
    radius: 0

    Component.onCompleted: {
        if (previewController) {
            previewController.previewItem = root
        }
    }
}
