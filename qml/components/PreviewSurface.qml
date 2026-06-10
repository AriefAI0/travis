import QtQuick

// Marks the preview rectangle used by the native D3D11 video overlay.

Rectangle {
    id: root

    property var previewController

    color: "#0f141a"
    border.color: "#3a4652"
    border.width: 1
    radius: 0

    function syncGeometryNow() {
        if (previewController) {
            previewController.syncPreviewGeometry()
        }
    }

    function syncGeometry() {
        syncGeometryNow()
        geometryFollowupTimer.restart()
    }

    Window.onWindowChanged: {
        if (Window.window && previewController) {
            previewController.previewItem = root
            syncGeometry()
        }
    }

    onXChanged: syncGeometry()
    onYChanged: syncGeometry()
    onWidthChanged: syncGeometry()
    onHeightChanged: syncGeometry()
    onVisibleChanged: syncGeometry()

    Connections {
        target: Window.window

        function onWidthChanged() {
            root.syncGeometry()
        }

        function onHeightChanged() {
            root.syncGeometry()
        }
    }

    Timer {
        id: geometryFollowupTimer

        interval: 16
        repeat: false
        onTriggered: root.syncGeometryNow()
    }
}
