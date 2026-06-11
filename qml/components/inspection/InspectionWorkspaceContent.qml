import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "avDock"
import "header"
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
            Label {
                Layout.fillWidth: true
                color: "#78818f"
                font.bold: true
                font.pixelSize: 10
                text: "INSPECTION TREE"
            },

            Label {
                Layout.fillWidth: true
                visible: inspectionContextViewModel.inspectionItems.length === 0
                color: "#78818f"
                text: inspectionContextViewModel.loading ? "Loading structure..." : "No inspection items."
                wrapMode: Text.Wrap
            },

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 4
                model: inspectionContextViewModel.inspectionItems

                delegate: Button {
                    width: ListView.view.width
                    height: 30
                    text: modelData.itemLabel
                    onClicked: inspectionContextViewModel.selectItem(modelData.itemId)

                    contentItem: ColumnLayout {
                        spacing: 1

                        Label {
                            Layout.fillWidth: true
                            color: modelData.itemId === inspectionContextViewModel.selectedItemId ? "#e6e8eb" : "#c9ced6"
                            font.pixelSize: 12
                            text: modelData.itemLabel
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            color: "#78818f"
                            font.pixelSize: 10
                            text: `${modelData.assetName} / ${modelData.componentName}`
                            elide: Text.ElideRight
                        }
                    }

                    background: Rectangle {
                        radius: 4
                        color: modelData.itemId === inspectionContextViewModel.selectedItemId
                            ? "#3b2227"
                            : (parent.hovered ? "#20242d" : "transparent")
                        border.color: modelData.itemId === inspectionContextViewModel.selectedItemId ? "#c1272d" : "transparent"
                        border.width: 1
                    }
                }
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
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#000000"

                PreviewSurface {
                    anchors.fill: parent
                    previewController: root.previewController
                }
            }
        ]

        rightSidebar: [
            Label {
                Layout.fillWidth: true
                color: "#78818f"
                font.bold: true
                font.pixelSize: 10
                text: "INSPECTION DETAILS"
            },

            Label {
                Layout.fillWidth: true
                color: "#e6e8eb"
                font.bold: true
                font.pixelSize: 14
                text: inspectionContextViewModel.selectedItemId > 0
                    ? inspectionContextViewModel.selectedItem.itemLabel
                    : "Select an inspection item"
                wrapMode: Text.Wrap
            },

            Label {
                Layout.fillWidth: true
                color: "#a8b0bd"
                text: inspectionContextViewModel.selectedItemId > 0
                    ? `${inspectionContextViewModel.selectedItem.assetName} / ${inspectionContextViewModel.selectedItem.componentName}`
                    : "Choose an item from the inspection tree to enable clipping."
                wrapMode: Text.Wrap
            },

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#303642"
            },

            Label {
                Layout.fillWidth: true
                color: "#78818f"
                font.bold: true
                font.pixelSize: 10
                text: "TASK TOOLS"
            },

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 6
                rowSpacing: 6

                Repeater {
                    model: ["GVI", "CVI", "MGI", "FMD"]

                    Button {
                        Layout.fillWidth: true
                        text: modelData
                        enabled: inspectionContextViewModel.selectedItemId > 0
                        onClicked: inspectionTypeField.text = modelData
                    }
                }
            },

            Label {
                Layout.fillWidth: true
                color: "#78818f"
                text: "Task tools are placeholders until detailed Electron task forms are migrated."
                wrapMode: Text.Wrap
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

                TextField {
                    id: inspectionTypeField

                    Layout.preferredWidth: 90
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
                        recordingViewModel.startInspectionClip(
                            inspectionContextViewModel.selectedItemId,
                            inspectionTypeField.text,
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
