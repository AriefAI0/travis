import QtQuick

// Marks the playback rectangle used by the native D3D11 video overlay.

Rectangle {
    id: root

    property var playbackController

    color: "#06090d"
    border.color: "#22313e"
    border.width: 1
    radius: 0

    function syncGeometryNow() {
        if (playbackController) {
            playbackController.syncPlaybackGeometry()
        }
    }

    function syncGeometry() {
        syncGeometryNow()
        geometryFollowupTimer.restart()
    }

    Component.onCompleted: {
        if (playbackController) {
            playbackController.playbackItem = root
            syncGeometry()
        }
    }

    Component.onDestruction: {
        if (playbackController) {
            playbackController.stopPlayback()
        }
    }

    Window.onWindowChanged: {
        if (Window.window && playbackController) {
            playbackController.playbackItem = root
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
