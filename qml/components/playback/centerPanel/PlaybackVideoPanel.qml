import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../.."

// Owns the native playback surface and compact transport controls.

Rectangle {
    id: root

    property var playbackViewModel
    property var playbackSurfaceController

    Layout.fillWidth: true
    Layout.preferredHeight: 300
    radius: 0
    color: "#101720"
    border.color: "#31404d"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Label {
            color: "#f4f7fa"
            font.pixelSize: 20
            text: root.playbackViewModel && root.playbackViewModel.selectedMasterVideoId > 0
                ? `Master Video #${root.playbackViewModel.selectedMasterVideoId}`
                : "No master video loaded"
        }

        Label {
            Layout.fillWidth: true
            color: "#9fb1bf"
            font.pixelSize: 12
            text: root.playbackViewModel && root.playbackViewModel.selectedMasterVideoPath.length > 0
                ? root.playbackViewModel.selectedMasterVideoPath
                : "Open playback from a project video list to load persisted media metadata."
            elide: Text.ElideMiddle
        }

        PlaybackSurface {
            Layout.fillWidth: true
            Layout.fillHeight: true
            playbackController: root.playbackSurfaceController

            Label {
                anchors.centerIn: parent
                visible: !root.playbackSurfaceController || !root.playbackSurfaceController.playbackActive
                color: "#758796"
                text: root.playbackViewModel && root.playbackViewModel.selectedMasterVideoPath.length > 0
                    ? "Press Play to start native playback"
                    : "Load a master video before playback"
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "Play"
                enabled: root.playbackSurfaceController &&
                    root.playbackViewModel &&
                    root.playbackViewModel.selectedMasterVideoPath.length > 0 &&
                    !root.playbackSurfaceController.playbackActive
                onClicked: root.playbackSurfaceController.startPlayback(
                    root.playbackViewModel.selectedMasterVideoPath
                )
            }

            Button {
                text: "Stop"
                enabled: root.playbackSurfaceController && root.playbackSurfaceController.playbackActive
                onClicked: root.playbackSurfaceController.stopPlayback()
            }

            Label {
                Layout.fillWidth: true
                color: "#9fb1bf"
                font.pixelSize: 12
                text: root.playbackSurfaceController && root.playbackSurfaceController.playbackActive
                    ? "Playback active"
                    : "Playback stopped"
            }
        }
    }
}
