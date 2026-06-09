import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../layouts"

// Hosts the first native inspection workspace around preview, AV dock, and recording controls.

Item {
    id: root

    property var navigation

    InspectionWorkspaceLayout {
        anchors.fill: parent
        title: "Inspection Workspace"
        subtitle: "Native Qt/QML shell for preview, source selection, and recording control"

        headerActions: [
            RecordingControls {
                recordingViewModel: recordingViewModel
            }
        ]

        controls: [
            StatusBanner {
                Layout.fillWidth: true
                message: recordingViewModel.lastError.length > 0
                    ? recordingViewModel.lastError
                    : previewSurfaceController.lastError
                error: true
            },

            StatusBanner {
                Layout.fillWidth: true
                message: recordingViewModel.statusMessage.length > 0
                    ? recordingViewModel.statusMessage
                    : playbackViewModel.statusMessage
                error: false
            },

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
        ]

        mainContent: [
            PreviewSurface {
                Layout.fillWidth: true
                Layout.fillHeight: true
                previewController: previewSurfaceController
            },

            PlaybackPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 140
                playbackViewModel: playbackViewModel
            }
        ]

        sideDock: [
            AvDock {
                Layout.fillHeight: true
                recordingViewModel: recordingViewModel
                previewController: previewSurfaceController
                sourceDiscoveryViewModel: sourceDiscoveryViewModel
                audioMeterViewModel: audioMeterViewModel
            }
        ]
    }

    Component.onCompleted: sourceDiscoveryViewModel.refreshAll()
}
