import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../layouts"

// Hosts the first native inspection workspace around preview, AV dock, and recording controls.

Item {
    id: root

    property var navigation
    property int projectId: 0

    function applyInspectionSession() {
        if (inspectionContextViewModel.sessionId > 0) {
            recordingViewModel.sessionId = inspectionContextViewModel.sessionId
        }
    }

    InspectionWorkspaceLayout {
        anchors.fill: parent
        title: "Inspection Workspace"
        subtitle: projectId > 0
            ? `Inspection workspace for ${inspectionContextViewModel.project.title || ("project " + projectId)}`
            : "Native Qt/QML shell for preview, source selection, and recording control"

        headerActions: [
            Button {
                text: "Projects"
                visible: root.navigation
                onClicked: root.navigation.goProjects()
            },

            Button {
                text: "Project"
                visible: root.navigation && root.projectId > 0
                onClicked: root.navigation.goProject(root.projectId)
            },

            RecordingControls {
                recordingViewModel: recordingViewModel
            }
        ]

        controls: [
            StatusBanner {
                Layout.fillWidth: true
                message: inspectionContextViewModel.lastError.length > 0
                    ? inspectionContextViewModel.lastError
                    : recordingViewModel.lastError.length > 0
                    ? recordingViewModel.lastError
                    : previewSurfaceController.lastError
                error: true
            },

            StatusBanner {
                Layout.fillWidth: true
                message: inspectionContextViewModel.statusMessage.length > 0
                    ? inspectionContextViewModel.statusMessage
                    : recordingViewModel.statusMessage.length > 0
                    ? recordingViewModel.statusMessage
                    : playbackViewModel.statusMessage
                error: false
            },

            RowLayout {
                Layout.fillWidth: true

                Label {
                    color: "#e3ebf3"
                    text: "Inspection Session"
                }

                ComboBox {
                    id: sessionCombo

                    Layout.preferredWidth: 260
                    model: inspectionContextViewModel.sessions
                    textRole: "name"
                    valueRole: "sessionId"
                    displayText: currentIndex >= 0 ? currentText : "Select session"
                    enabled: root.projectId > 0 && inspectionContextViewModel.sessions.length > 0

                    onActivated: {
                        if (inspectionContextViewModel.selectSession(currentValue)) {
                            root.applyInspectionSession()
                        }
                    }
                }

                TextField {
                    id: sessionNameField

                    Layout.preferredWidth: 220
                    placeholderText: "New session name"
                    enabled: root.projectId > 0
                    onAccepted: createSessionButton.clicked()
                }

                Button {
                    id: createSessionButton

                    text: "Create Session"
                    enabled: root.projectId > 0 && !inspectionContextViewModel.loading
                    onClicked: {
                        if (inspectionContextViewModel.createSession(sessionNameField.text)) {
                            sessionNameField.clear()
                            root.applyInspectionSession()
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    color: "#9fb1bf"
                    text: inspectionContextViewModel.sessionId > 0
                        ? `Active session: ${inspectionContextViewModel.selectedSession.name}`
                        : "Create or select a session before recording"
                    elide: Text.ElideRight
                }
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
                    enabled: false
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

    onProjectIdChanged: {
        if (projectId > 0 && inspectionContextViewModel.loadProject(projectId)) {
            root.applyInspectionSession()
        }
    }

    Component.onCompleted: {
        sourceDiscoveryViewModel.refreshAll()
        if (projectId > 0 && inspectionContextViewModel.loadProject(projectId)) {
            root.applyInspectionSession()
        }
    }

    Connections {
        target: inspectionContextViewModel

        function onSessionChanged() {
            root.applyInspectionSession()
        }
    }
}
