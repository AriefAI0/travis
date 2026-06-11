import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "avDock"
import "bottomBar"
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

    function defaultClipOutputPath() {
        const masterPath = recordingViewModel.activeMasterVideoPath || recordingViewModel.outputPath
        if (!masterPath || masterPath.length === 0) {
            return ""
        }

        const normalizedPath = masterPath.replace(/\\/g, "/")
        const slashIndex = normalizedPath.lastIndexOf("/")
        const directory = slashIndex >= 0 ? normalizedPath.slice(0, slashIndex + 1) : ""
        const timestamp = Date.now()
        const itemId = inspectionContextViewModel.selectedItemId
        const typeName = inspectionDetailsPanel.inspectionTypeText.trim().toLowerCase()

        return `${directory}clip-${itemId}-${typeName}-${timestamp}.mkv`
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
                recordingViewModel: root.recordingVm
                onStartClipRequested: function(inspectionTypeName) {
                    recordingViewModel.startInspectionClip(
                        inspectionContextViewModel.selectedItemId,
                        inspectionTypeName,
                        root.defaultClipOutputPath()
                    )
                }
                onStopClipRequested: function(clipId) {
                    recordingViewModel.stopInspectionClip(clipId)
                }
            }
        ]

        bottomContent: [
            InspectionEventPlaceholder {
                Layout.fillWidth: true
                Layout.fillHeight: true
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
