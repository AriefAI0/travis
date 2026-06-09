import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Presents a small persisted playback summary from the playback viewmodel.

Rectangle {
    id: root

    property var playbackViewModel

    radius: 0
    color: "#162029"
    border.color: "#31404d"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        Label {
            text: "Playback"
            font.pixelSize: 18
            color: "#f4f7fa"
        }

        Label {
            Layout.fillWidth: true
            color: "#b5c2cd"
            text: playbackViewModel && playbackViewModel.selectedMasterVideoPath.length > 0
                ? "Master: " + playbackViewModel.selectedMasterVideoPath
                : "No master video selected"
            wrapMode: Text.Wrap
        }

        Label {
            Layout.fillWidth: true
            color: "#b5c2cd"
            text: playbackViewModel && playbackViewModel.selectedClipPath.length > 0
                ? "Clip: " + playbackViewModel.selectedClipPath
                : "No clip selected"
            wrapMode: Text.Wrap
        }

        Label {
            Layout.fillWidth: true
            visible: playbackViewModel && playbackViewModel.statusMessage.length > 0
            color: "#8fd0ff"
            text: playbackViewModel ? playbackViewModel.statusMessage : ""
            wrapMode: Text.Wrap
        }

        Label {
            Layout.fillWidth: true
            visible: playbackViewModel && playbackViewModel.lastError.length > 0
            color: "#f09999"
            text: playbackViewModel ? playbackViewModel.lastError : ""
            wrapMode: Text.Wrap
        }
    }
}
