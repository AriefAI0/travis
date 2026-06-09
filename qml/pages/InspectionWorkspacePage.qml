import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"

// Hosts the first native inspection workspace around preview, AV dock, and recording controls.

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0d1117" }
            GradientStop { position: 1.0; color: "#19222b" }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 4

                Label {
                    text: "Inspection Workspace"
                    font.pixelSize: 28
                    color: "#f6f8fb"
                }

                Label {
                    color: "#9fb1bf"
                    text: "Native Qt/QML shell for preview, source selection, and recording control"
                }
            }

            Item {
                Layout.fillWidth: true
            }

            RecordingControls {
                recordingViewModel: recordingViewModel
            }
        }

        StatusBanner {
            Layout.fillWidth: true
            message: recordingViewModel.lastError.length > 0
                ? recordingViewModel.lastError
                : previewSurfaceController.lastError
            error: true
        }

        StatusBanner {
            Layout.fillWidth: true
            message: recordingViewModel.statusMessage.length > 0
                ? recordingViewModel.statusMessage
                : playbackViewModel.statusMessage
            error: false
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                color: "#e3ebf3"
                text: "Recording ID"
            }

            TextField {
                Layout.preferredWidth: 180
                text: recordingViewModel.recordingId
                placeholderText: "recording-001"
                onTextChanged: recordingViewModel.recordingId = text
            }

            Label {
                color: "#e3ebf3"
                text: "Session ID"
            }

            TextField {
                Layout.preferredWidth: 120
                text: recordingViewModel.sessionId > 0 ? recordingViewModel.sessionId.toString() : ""
                placeholderText: "1"
                inputMethodHints: Qt.ImhDigitsOnly
                onTextChanged: recordingViewModel.sessionId = text.length > 0 ? Number(text) : 0
            }

            Label {
                color: "#e3ebf3"
                text: "Output"
            }

            TextField {
                Layout.fillWidth: true
                text: recordingViewModel.outputPath
                placeholderText: "C:/recordings/session-001/master.mkv"
                onTextChanged: recordingViewModel.outputPath = text
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 3
                spacing: 12

                PreviewSurface {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    previewController: previewSurfaceController
                }

                PlaybackPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 140
                    playbackViewModel: playbackViewModel
                }
            }

            AvDock {
                Layout.fillHeight: true
                Layout.preferredWidth: 360
                recordingViewModel: recordingViewModel
                previewController: previewSurfaceController
                sourceDiscoveryViewModel: sourceDiscoveryViewModel
                audioMeterViewModel: audioMeterViewModel
            }
        }
    }

    Component.onCompleted: sourceDiscoveryViewModel.refreshAll()
}
