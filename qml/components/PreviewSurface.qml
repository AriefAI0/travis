import QtQuick

// Marks the preview rectangle used by the native D3D11 video overlay.

Rectangle {
    id: root

    property var previewController

    color: "#0f141a"
    border.color: "#3a4652"
    border.width: 1
    radius: 0

    Window.onWindowChanged: {
        if (Window.window && previewController) {
            previewController.previewItem = root
        }
    }
}
