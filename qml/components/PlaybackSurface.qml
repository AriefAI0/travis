import QtQuick

// Marks the playback rectangle used by the native D3D11 video overlay.

Rectangle {
    id: root

    property var playbackController

    color: "#06090d"
    border.color: "#22313e"
    border.width: 1
    radius: 0

    Component.onCompleted: {
        if (playbackController) {
            playbackController.playbackItem = root
        }
    }

    Component.onDestruction: {
        if (playbackController) {
            playbackController.stopPlayback()
        }
    }
}
