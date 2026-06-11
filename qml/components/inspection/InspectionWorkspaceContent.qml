import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "avDock"
import "centerPanel"
import "header"
import "leftPanel"
import "rightPanel"
import "../../layouts"

// Composes the inspection workspace slots while keeping backend work in viewmodels.

Item {
    id: root

    property var navigation
    property int projectId: 0
    property var recordingVm: recordingViewModel
    property var playbackVm: playbackViewModel
    property var previewController: previewSurfaceController
    property var sourceDiscoveryVm: sourceDiscoveryViewModel
    property var audioMeterVm: audioMeterViewModel
    property bool completed: false

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

    function scheduleInspectionContextLoad() {
        if (root.completed && root.projectId > 0) {
            Qt.callLater(loadInspectionContext)
        }
    }

    function loadInspectionContext() {
        if (root.projectId > 0 && inspectionContextViewModel.loadProject(root.projectId)) {
            root.applyInspectionSession()
        }
    }

    InspectionWorkspaceLayout {
        anchors.fill: parent
        title: inspectionContextViewModel.project.title || "Inspection Workspace"
        subtitle: "Inspection workspace"

        headerNavigation: [
            Button {
                Layout.alignment: Qt.AlignLeft
                flat: true
                implicitHeight: 24
                text: "< Project"
                enabled: root.navigation
                onClicked: root.navigation && root.projectId > 0
                    ? root.navigation.goProject(root.projectId)
                    : root.navigation.goProjects()
            },

            Label {
                Layout.fillWidth: true
                color: "#78818f"
                font.pixelSize: 10
                text: "INSPECTION WORKSPACE"
                elide: Text.ElideRight
            }
        ]

        headerStatus: [
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Rectangle {
                    Layout.preferredWidth: 8
                    Layout.preferredHeight: 8
                    radius: 4
                    color: recordingViewModel.recordingActive
                        ? (recordingViewModel.paused ? "#d8a536" : "#c1272d")
                        : "#78818f"
                }

                Label {
                    color: "#a8b0bd"
                    font.pixelSize: 11
                    text: recordingViewModel.recordingActive
                        ? (recordingViewModel.paused ? "Paused" : "Recording")
                        : "Idle"
                }

                Label {
                    color: "#78818f"
                    font.pixelSize: 11
                    text: inspectionContextViewModel.sessionId > 0
                        ? `Session ${inspectionContextViewModel.sessionId}`
                        : "No session"
                }

                Item {
                    Layout.fillWidth: true
                }

                RecordingControl {
                    recordingViewModel: root.recordingVm
                }
            }
        ]

        leftSidebar: [
            InspectionTreePanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                inspectionContextViewModel: inspectionContextViewModel
            }
        ]

        leftBottom: [
            InspectionAvDock {
                Layout.fillWidth: true
                Layout.fillHeight: true
                recordingViewModel: root.recordingVm
                previewController: root.previewController
                sourceDiscoveryViewModel: root.sourceDiscoveryVm
                audioMeterViewModel: root.audioMeterVm
            }
        ]

        centerContent: [
            InspectionPreviewPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                previewController: root.previewController
            }
        ]

        rightSidebar: [
            InspectionDetailsPanel {
                id: inspectionDetailsPanel

                Layout.fillWidth: true
                Layout.fillHeight: true
                inspectionContextViewModel: inspectionContextViewModel
            }
        ]

        bottomContent: [
            Label {
                Layout.fillWidth: true
                color: "#78818f"
                font.bold: true
                font.pixelSize: 10
                text: "RECORDER / RESERVED"
            },

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

                ComboBox {
                    Layout.preferredWidth: 220
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

                    Layout.preferredWidth: 180
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
            },

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.preferredWidth: 90
                    color: "#a8b0bd"
                    text: inspectionDetailsPanel.inspectionTypeText
                    elide: Text.ElideRight
                }

                TextField {
                    id: clipOutputPathField

                    Layout.fillWidth: true
                    placeholderText: "Clip output path"
                }

                Button {
                    text: "Start Clip"
                    enabled: root.canStartClip() &&
                        inspectionDetailsPanel.inspectionTypeText.trim().length > 0 &&
                        clipOutputPathField.text.trim().length > 0
                    onClicked: {
                        recordingViewModel.startInspectionClip(
                            inspectionContextViewModel.selectedItemId,
                            inspectionDetailsPanel.inspectionTypeText,
                            clipOutputPathField.text
                        )
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

                TextField {
                    Layout.preferredWidth: 180
                    text: recordingViewModel.recordingId
                    placeholderText: "Recording ID"
                    onTextChanged: recordingViewModel.recordingId = text
                }

                TextField {
                    Layout.fillWidth: true
                    text: recordingViewModel.outputPath
                    placeholderText: "C:/recordings/session-001/master.mkv"
                    onTextChanged: recordingViewModel.outputPath = text
                }
            },

            PlaybackPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                playbackViewModel: root.playbackVm
            }
        ]
    }

    onProjectIdChanged: root.scheduleInspectionContextLoad()

    Component.onCompleted: {
        root.completed = true
        root.scheduleInspectionContextLoad()
    }

    Component.onDestruction: {
        if (root.previewController) {
            root.previewController.stopPreview()
        }

        if (root.audioMeterVm) {
            root.audioMeterVm.stopAll()
        }
    }

    Connections {
        target: inspectionContextViewModel

        function onSessionChanged() {
            root.applyInspectionSession()
        }
    }
}
