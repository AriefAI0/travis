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
    property var recordingVm: recordingViewModel
    property var playbackVm: playbackViewModel
    property var previewController: previewSurfaceController
    property var sourceDiscoveryVm: sourceDiscoveryViewModel
    property var audioMeterVm: audioMeterViewModel

    function applyInspectionSession() {
        if (inspectionContextViewModel.sessionId > 0) {
            recordingViewModel.sessionId = inspectionContextViewModel.sessionId
        }
    }

    function canStartClip() {
        return recordingViewModel.recordingActive &&
            !recordingViewModel.paused &&
            inspectionContextViewModel.sessionId > 0 &&
            inspectionContextViewModel.selectedItemId > 0 &&
            recordingViewModel.activeClipId === 0
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
                enabled: root.navigation
                onClicked: root.navigation.goProjects()
            },

            Button {
                text: "Project"
                enabled: root.navigation && root.projectId > 0
                onClicked: root.navigation.goProject(root.projectId)
            },

            RecordingControls {
                recordingViewModel: root.recordingVm
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
                    text: "Inspection Item"
                }

                ComboBox {
                    id: inspectionItemCombo

                    Layout.preferredWidth: 360
                    model: inspectionContextViewModel.inspectionItems
                    textRole: "displayName"
                    valueRole: "itemId"
                    displayText: currentIndex >= 0 ? currentText : "Select item"
                    enabled: inspectionContextViewModel.inspectionItems.length > 0

                    onActivated: inspectionContextViewModel.selectItem(currentValue)
                }

                TextField {
                    id: inspectionTypeField

                    Layout.preferredWidth: 120
                    text: "GVI"
                    placeholderText: "Type"
                }

                TextField {
                    id: clipOutputPathField

                    Layout.fillWidth: true
                    placeholderText: "Clip output path"
                }

                Button {
                    text: "Start Clip"
                    enabled: root.canStartClip() &&
                        inspectionTypeField.text.trim().length > 0 &&
                        clipOutputPathField.text.trim().length > 0
                    onClicked: {
                        if (recordingViewModel.startInspectionClip(
                                inspectionContextViewModel.selectedItemId,
                                inspectionTypeField.text,
                                clipOutputPathField.text
                            )) {}
                    }
                }

                Button {
                    text: "Stop Clip"
                    enabled: recordingViewModel.activeClipId > 0
                    onClicked: recordingViewModel.stopInspectionClip(recordingViewModel.activeClipId)
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
                playbackViewModel: root.playbackVm
            }
        ]

        sideDock: [
            AvDock {
                Layout.fillHeight: true
                recordingViewModel: root.recordingVm
                previewController: root.previewController
                sourceDiscoveryViewModel: root.sourceDiscoveryVm
                audioMeterViewModel: root.audioMeterVm
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

    Component.onDestruction: {
        if (previewSurfaceController) {
            previewSurfaceController.stopPreview()
        }

        if (audioMeterViewModel) {
            audioMeterViewModel.stopAll()
        }
    }

    Connections {
        target: inspectionContextViewModel

        function onSessionChanged() {
            root.applyInspectionSession()
        }
    }
}
